//
// Created by wilbert on 2026/1/26.
//

#include "VideoClip.hpp"
#include "common/Log.hpp"
#include <thread>
#include <chrono>

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libswscale/swscale.h"
#include "ffmpeg/libavutil/imgutils.h"
}

namespace face {
    VideoClip::VideoClip(const std::string &videoFile, std::shared_ptr<ComponentId> trackId): mVideoFile(videoFile), Clip(ClipType::Video, trackId) {
        LOGE("VideoClip::%s[%d]", __FUNCTION__, mTrackId->getId() );
        auto maxSize = 6;
        mDataQueue = std::make_shared<LimitQueue<PixelData>>();
        mDataQueue->setLimitPolicy(LimitPolicy::WaitWhenBusy);
        mDataQueue->setMaxSize(maxSize);

        mRecycleQueue = std::make_shared<LimitQueue<PixelData>>();
        mRecycleQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
        mRecycleQueue->setMaxSize(maxSize);

        mFrameBuffers = std::make_shared<LimitQueue<FrameBuffer>>();
        mFrameBuffers->setLimitPolicy(LimitPolicy::DropWhenBusy);
        mFrameBuffers->setMaxSize(4);

        mThread = std::make_shared<LoopThread>("VideoClip");
        mThread->setLoopMode(LoopMode::REQUEST);
    }

    VideoClip::~VideoClip() {
        LOGE("VideoClip::%s[%d]", __FUNCTION__, mTrackId->getId());
        mThread->setOnLoopListener(nullptr);
        mThread->stop();
    }

    bool VideoClip::render(uint64_t timeStamp, const std::shared_ptr<RenderTarget>& renderTarget) {
        if (!mPixelRender) {
            LOGE("VideoClip::%s[%d], time:%llu when pixelRender null", __FUNCTION__, mTrackId->getId(), static_cast<unsigned long long>(timeStamp));
            mPixelRender = std::make_shared<PixelRender>();
        }
        auto data = getData(timeStamp);
        if (data && renderTarget) {
            std::lock_guard<std::mutex> lk(mMutex);
            if (!mOutputRender) {
                return mPixelRender->render(data, renderTarget) == Error::None;
            } else {
                auto target = getCachedTarget(renderTarget);
                mPixelRender->render(data, target);
                for (auto& render: mEffectRenders) {
                    auto tempTarget = getCachedTarget(renderTarget);
                    render->render(target->getFboTexture(), tempTarget);
                    mFrameBuffers->push(target);
                    target = tempTarget;
                }
                auto res = mOutputRender->render(target->getFboTexture(), renderTarget); // 最后一个渲染器绘制到输入的renderTarget上
                mFrameBuffers->push(target);
                return res == Error::None;
            }
        }
        return false;
    }

    void VideoClip::start(uint64_t start, uint64_t end) {
        LOGE("VideoClip::%s[%d]", __FUNCTION__, mTrackId->getId());
        std::weak_ptr<VideoClip> weakPtr(shared_from_this());
        mThread->setOnStartListener([weakPtr] () {
            if (auto clip = weakPtr.lock()) {
                clip->onStart();
            }
        });
        mThread->setOnLoopListener([weakPtr] (uint64_t requestId) {
            if (auto clip = weakPtr.lock()) {
                clip->onLoop(requestId);
            }
        });
        mThread->setOnStopListener([weakPtr] () {
            if (auto clip = weakPtr.lock()) {
                clip->onStop();
            }
        });
        setDuration(start, end);
        mThread->start();
        seek(1);
    }

    void VideoClip::flush() {
        while(auto data = mDataQueue->pop()) { // 清理缓存
            mRecycleQueue->push(data);
        }
    }

    void VideoClip::seek(uint64_t timeStamp) {
        flush();
        auto thread = mThread;
        if (thread) {
            thread->requestLoop(timeStamp);
        }
    }

    void VideoClip::onStart() {
        LOGE("VideoClip::onStart[%d] %s", mTrackId->getId(), mVideoFile.c_str());
        av_register_all();
        int ret = 0;
        if ((ret = avformat_open_input(&mFormatCtx, mVideoFile.c_str(), nullptr, nullptr)) != 0) {
            LOGE("VideoClip::onStart[%d] open input failed, ret:%s", mTrackId->getId(),av_err2str(ret));
            return;
        }
        if (avformat_find_stream_info(mFormatCtx, nullptr) < 0) {
            LOGE("VideoClip::onStart[%d] find stream info failed", mTrackId->getId());
            return;
        }

        mVideoStreamIndex = -1;
        for (unsigned int i = 0; i < mFormatCtx->nb_streams; i++) {
            if (mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                mVideoStreamIndex = i;
                break;
            }
        }

        if (mVideoStreamIndex == -1) {
            LOGE("VideoClip::onStart[%d] find video stream failed", mTrackId->getId());
            return;
        }

        AVCodecParameters* codecPar = mFormatCtx->streams[mVideoStreamIndex]->codecpar;
        mCodec = avcodec_find_decoder(codecPar->codec_id);
        if (!mCodec) {
            LOGE("VideoClip::onStart[%d] find decoder failed", mTrackId->getId());
            return;
        }
        LOGE("VideoClip::%s[%d] success, mCodecCtx:%p, mCodec:%p", __FUNCTION__, mTrackId->getId(), mCodecCtx, mCodec);
        mCodecCtx = avcodec_alloc_context3(mCodec);
        if (!mCodecCtx) {
            LOGE("VideoClip::onStart[%d] alloc codec context failed", mTrackId->getId());
            return;
        }

        if (avcodec_parameters_to_context(mCodecCtx, codecPar) < 0) {
            LOGE("VideoClip::onStart[%d] parameters to context failed", mTrackId->getId());
            return;
        }

        if (avcodec_open2(mCodecCtx, mCodec, nullptr) < 0) {
            LOGE("VideoClip::onStart[%d] open codec failed", mTrackId->getId());
            return;
        }

        mFrame = av_frame_alloc();
        mPacket = av_packet_alloc();
        LOGE("VideoClip::%s[%d] success, mCodecCtx:%p", __FUNCTION__, mTrackId->getId(), mCodecCtx);
    }

    void VideoClip::onLoop(uint64_t timestamp) {
        if (!mFormatCtx || !mCodecCtx) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return;
        }

        int ret = av_read_frame(mFormatCtx, mPacket);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                av_seek_frame(mFormatCtx, mVideoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
            }
            return;
        }

        auto thread = mThread;
        if (thread) thread->requestLoop();

        if (mPacket->stream_index == mVideoStreamIndex) {
            ret = avcodec_send_packet(mCodecCtx, mPacket);
            if (ret < 0) {
                LOGE("VideoClip::onLoop[%d] send packet error", mTrackId->getId());
                av_packet_unref(mPacket);
                return;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(mCodecCtx, mFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    LOGE("VideoClip::onLoop[%d] eof end", mTrackId->getId());
                    break;
                } else if (ret < 0) {
                    LOGE("VideoClip::onLoop[%d] receive frame error", mTrackId->getId());
                    break;
                }

                auto pixelData = getEmptyData();
                int width = mCodecCtx->width;
                int height = mCodecCtx->height;

                if (!mSwsCtx) {
                    mSwsCtx = sws_getContext(width, height, mCodecCtx->pix_fmt,
                                             width, height, AV_PIX_FMT_YUV420P,
                                             SWS_BILINEAR, nullptr, nullptr, nullptr);
                }

                int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
                pixelData->allocate(size);
                pixelData->setFormat(PixelFormat::I420P);
                pixelData->setResolution(width, height);
                pixelData->setPixelSize(size);
                
                if (mFrame->pts != AV_NOPTS_VALUE) {
                    AVRational time_base = mFormatCtx->streams[mVideoStreamIndex]->time_base;
                    uint64_t pts_ms = (uint64_t)(mFrame->pts * av_q2d(time_base) * 1000);
                    pixelData->setTimestamp(mDuration.start + pts_ms);
                }

                uint8_t* dstData[4];
                int dstLinesize[4];
                av_image_fill_arrays(dstData, dstLinesize, pixelData->getWritablePixels(), AV_PIX_FMT_YUV420P, width, height, 1);
                sws_scale(mSwsCtx, mFrame->data, mFrame->linesize, 0, height, dstData, dstLinesize);
                LOGE("VideoClip::%s[%d] onPushFrame, %dx%d, pts:%llu, clipStartAt:%llu", __FUNCTION__, mTrackId->getId(), width, height, static_cast<unsigned long long>(pixelData->getTimestamp()), static_cast<unsigned long long>(mDuration.start));
                mDataQueue->push(pixelData);
                onInspect(pixelData);
            }
        }
        av_packet_unref(mPacket);
    }

    void VideoClip::onStop() {
        LOGE("VideoClip::onStop[%d]", mTrackId->getId());
        if (mSwsCtx) {
            sws_freeContext(mSwsCtx);
            mSwsCtx = nullptr;
        }
        if (mFrame) {
            av_frame_free(&mFrame);
            mFrame = nullptr;
        }
        if (mPacket) {
            av_packet_free(&mPacket);
            mPacket = nullptr;
        }
        if (mCodecCtx) {
            avcodec_free_context(&mCodecCtx);
            mCodecCtx = nullptr;
        }
        if (mFormatCtx) {
            avformat_close_input(&mFormatCtx);
            mFormatCtx = nullptr;
        }
        mDataQueue->clear();
    }

    std::shared_ptr<PixelData> VideoClip::getEmptyData() {
        auto result = mRecycleQueue->pop();
        if (!result) {
            result = std::make_shared<PixelData>();
        }
        return result;
    }

    bool VideoClip::releaseRender() {
        if (mPixelRender) {
            mPixelRender->destroy();
            mPixelRender.reset();
            return true;
        }
        return false;
    }

    std::shared_ptr<PixelData> VideoClip::getData(uint64_t timeStamp) {
        auto condition = [timeStamp] (const std::shared_ptr<PixelData>& pixelData) -> std::shared_ptr<PixelData> {
            if (pixelData && pixelData->getTimestamp() <= timeStamp) {
                return pixelData;
            } else {
                return nullptr;
            }
        };
        std::shared_ptr<PixelData> data = nullptr;
        while(auto temp = mDataQueue->pop(condition)) {
            if (data) mRecycleQueue->push(data);
            data = temp;
        }
        return data;
    }

    std::shared_ptr<FrameBuffer>
    VideoClip::getCachedTarget(const std::shared_ptr<RenderTarget> &displayTarget) {
        auto condition = [displayTarget] (const std::shared_ptr<FrameBuffer>& frameBuffer) -> std::shared_ptr<FrameBuffer> {
            if (!frameBuffer) {
                auto target = std::make_shared<FrameBuffer>();
                displayTarget->copyTo(target);
                return target;
            }
            if (frameBuffer->getWidth() != displayTarget->getWidth() || frameBuffer->getHeight() != displayTarget->getHeight()) {
                return nullptr;
            }
            return frameBuffer;
        };
        auto target = mFrameBuffers->pop(condition);
        if (!target) { // renderTarget已经发生变更，需要立即清理旧数据
            mFrameBuffers->clear();
            target = std::make_shared<FrameBuffer>();
            displayTarget->copyTo(target);
        }
        return target;
    }

    void VideoClip::addRender(const std::shared_ptr<Render<face::Texture>> &render, const std::shared_ptr<Inspector<PixelData>>& inspector) {
        std::lock_guard<std::mutex> lk(mMutex);
        if (mOutputRender) {
            mEffectRenders.push_back(mOutputRender);
        }
        mOutputRender = render;
        if (inspector) mInspectors.push_back(inspector);
    }

    void VideoClip::onInspect(const std::shared_ptr<PixelData> &data) {
        std::lock_guard<std::mutex> lk(mMutex);
        for (auto& inspector: mInspectors) {
            inspector->inspect(data);
        }
    }
} // face
