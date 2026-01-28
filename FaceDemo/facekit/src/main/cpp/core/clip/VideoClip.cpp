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
    bool VideoClip::render(uint64_t timeStamp) {
        if (!mPixelRender) {
            LOGE("VideoClip::%s, time:%llu when pixelRender null", __FUNCTION__, timeStamp);
            return false;
        }
        bool hasFrame = false;
        auto condition = [timeStamp] (const std::shared_ptr<RenderData<PixelData>>& renderData) -> bool {
            if (!renderData) return false;
            auto& pixelData = renderData->data;
            if (pixelData->getTimestamp() <= timeStamp) {
                return true;
            } else {
                return false;
            }
        };
        while(auto data = mDataQueue->pop(condition)) {
            LOGE("VideoClip::%s going to render pts:%llu", __FUNCTION__, data->data->getTimestamp());
            auto res = mPixelRender->render(data);
            mRecycleQueue->push(data);
            hasFrame = true;
        }
        return hasFrame;
    }

    VideoClip::VideoClip(const std::string &videoFile): mVideoFile(videoFile), Clip(ClipType::Video) {
        LOGE("VideoClip::%s", __FUNCTION__ );
        auto maxSize = 6;
        mDataQueue = std::make_shared<LimitQueue<RenderData<PixelData>>>();
        mDataQueue->setLimitPolicy(LimitPolicy::WaitWhenBusy);
        mDataQueue->setMaxSize(maxSize);

        mRecycleQueue = std::make_shared<LimitQueue<RenderData<PixelData>>>();
        mRecycleQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
        mRecycleQueue->setMaxSize(maxSize);

        mThread = std::make_shared<LoopThread>("VideoClip");
        mThread->setLoopMode(LoopMode::REQUEST);
    }

    VideoClip::~VideoClip() {
        LOGE("VideoClip::%s", __FUNCTION__ );
        mThread->setOnLoopListener(nullptr);
        mThread->stop();
    }

    void VideoClip::start(uint64_t start, uint64_t end) {
        LOGE("VideoClip::%s", __FUNCTION__ );
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
        LOGE("VideoClip::onStart %s", mVideoFile.c_str());
        av_register_all();
        int ret = 0;
        if ((ret = avformat_open_input(&mFormatCtx, mVideoFile.c_str(), nullptr, nullptr)) != 0) {
            LOGE("VideoClip::onStart open input failed, ret:%s", av_err2str(ret));
            return;
        }
        if (avformat_find_stream_info(mFormatCtx, nullptr) < 0) {
            LOGE("VideoClip::onStart find stream info failed");
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
            LOGE("VideoClip::onStart find video stream failed");
            return;
        }

        AVCodecParameters* codecPar = mFormatCtx->streams[mVideoStreamIndex]->codecpar;
        mCodec = avcodec_find_decoder(codecPar->codec_id);
        if (!mCodec) {
            LOGE("VideoClip::onStart find decoder failed");
            return;
        }
        LOGE("VideoClip::%s success, mCodecCtx:%p, mCodec:%p", __FUNCTION__, mCodecCtx, mCodec);
        mCodecCtx = avcodec_alloc_context3(mCodec);
        if (!mCodecCtx) {
            LOGE("VideoClip::onStart alloc codec context failed");
            return;
        }

        if (avcodec_parameters_to_context(mCodecCtx, codecPar) < 0) {
            LOGE("VideoClip::onStart parameters to context failed");
            return;
        }

        if (avcodec_open2(mCodecCtx, mCodec, nullptr) < 0) {
            LOGE("VideoClip::onStart open codec failed");
            return;
        }

        mFrame = av_frame_alloc();
        mPacket = av_packet_alloc();
        LOGE("VideoClip::%s success, mCodecCtx:%p", __FUNCTION__, mCodecCtx);
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
                LOGE("VideoClip::onLoop send packet error");
                av_packet_unref(mPacket);
                return;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(mCodecCtx, mFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    LOGE("VideoClip::onLoop eof end");
                    break;
                } else if (ret < 0) {
                    LOGE("VideoClip::onLoop receive frame error");
                    break;
                }

                auto renderData = getEmptyData();
                renderData->scaleType = ScaleType::FitCenter;
                int width = mCodecCtx->width;
                int height = mCodecCtx->height;

                if (!mSwsCtx) {
                    mSwsCtx = sws_getContext(width, height, mCodecCtx->pix_fmt,
                                             width, height, AV_PIX_FMT_YUV420P,
                                             SWS_BILINEAR, nullptr, nullptr, nullptr);
                }

                int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
                auto& pixelData = renderData->data;
                if (!pixelData) {
                    pixelData = std::make_shared<PixelData>();
                }
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
//                if (pixelData->getTimestamp() < 1000) {
//                    FILE* file = fopen("/data/data/com.jarvis.facedemo/files/test_start.yuv", "w+");
//                    fwrite(pixelData->getPixels(), pixelData->getPixelSize(), 1, file);
//                    fclose(file);
//                }
//                if (pixelData->getTimestamp() > 10070) {
//                    FILE* file = fopen("/data/data/com.jarvis.facedemo/files/test_end.yuv", "w+");
//                    fwrite(pixelData->getPixels(), pixelData->getPixelSize(), 1, file);
//                    fclose(file);
//                }
                LOGE("VideoClip::%s onPushFrame, %dx%d, pts:%llu, clipStartAt:%llu", __FUNCTION__, width, height, pixelData->getTimestamp(), mDuration.start);
                mDataQueue->push(renderData);
            }
        }
        av_packet_unref(mPacket);
    }

    void VideoClip::onStop() {
        LOGE("VideoClip::onStop");
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

    std::shared_ptr<RenderData<PixelData>> VideoClip::getEmptyData() {
        auto result = mRecycleQueue->pop();
        if (!result) {
            result = std::make_shared<RenderData<PixelData>>();
        }
        return result;
    }

    void VideoClip::onSizeChanged(uint32_t width, uint32_t height) {
        if (mPixelRender) {
            mPixelRender->destroy();
        }
        mPixelRender = std::make_shared<PixelRender>();
        mPixelRender->resize(width, height);
    }
} // face
