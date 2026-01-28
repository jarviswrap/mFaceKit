//
// Created by wilbert on 2026/1/26.
//

#include "VideoTrack.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLEnvironment.hpp"
#include "render/utils/FrameBuffer.hpp"
#include "render/utils/OpenGLUtils.hpp"
#include "core/clip/VideoClip.hpp"
#include "common/Log.hpp"

namespace face {

    VideoTrack::VideoTrack(): Track(TrackType::Video) {
        size_t maxSize = 5;
        mDataQueue = std::make_shared<LimitQueue<FrameBuffer>>();
        mDataQueue->setLimitPolicy(LimitPolicy::WaitWhenBusy);
        mDataQueue->setMaxSize(maxSize);

        mRecycleQueue = std::make_shared<LimitQueue<FrameBuffer>>();
        mRecycleQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
        mRecycleQueue->setMaxSize(maxSize);
    }

    void VideoTrack::onRender(std::shared_ptr<face::Clip> clip, uint64_t timeStamp) {
        if (!clip) {
//            LOGE("VideoTrack::%s no clip, timeStamp:%llu", __FUNCTION__, timeStamp);
            return;
        }
        if (initRenderEnv()) {
            if (mSizeChanged) {
                mDataQueue->clear();
                mRecycleQueue->clear();

                uint32_t width = 0;
                uint32_t height = 0;
                mTrackSize.getSize(width, height);
                LOGE("VideoTrack::%s %dx%d", __FUNCTION__, width, height);
                for(auto& c: mClips) {
                    if (clip->getClipType() == ClipType::Video) {
                        auto videoClip = std::static_pointer_cast<VideoClip>(c);
                        videoClip->onSizeChanged(width, height);
                    }
                }
                mSizeChanged = false;
            }

            auto size = mTrackSize;
            auto condition = [timeStamp, clip, size] (const std::shared_ptr<FrameBuffer>& frameBuffer) -> bool {
                if (!frameBuffer) return false;
                if (!frameBuffer->isInitialized()) {
                    uint32_t width = 0;
                    uint32_t height = 0;
                    size.getSize(width, height);
                    frameBuffer->init(width, height);
                }
                frameBuffer->bind();
                LOGE("VideoTrack::%s renderTrace start render to textureId:%d, timeStamp:%llu", __FUNCTION__, frameBuffer->getFboTexture()->getTextureId(), timeStamp);
                return clip->render(timeStamp);
            };

            auto result = mRecycleQueue->pop(condition);
            if (result) {
                LOGE("VideoTrack::%s renderTrace clipRendered to textureId:%d, timeStamp:%llu success", __FUNCTION__, result->getFboTexture()->getTextureId(), timeStamp);
                OpenGLUtils::flush();
                mDataQueue->push(result);
            } else {
                LOGE("VideoTrack::%s renderTrace timeStamp:%llu failed", __FUNCTION__, timeStamp);
            }
        }
    }

    void VideoTrack::onThreadStart() {
        initRenderEnv();
    }

    void VideoTrack::onThreadStop() {
        if (mGLEnvironment) {
            mGLEnvironment->destroyEGLSurface();
            mGLEnvironment->destroyEGLContext();
            mGLEnvironment.reset();
        }
    }

    bool VideoTrack::initRenderEnv() {
        if (!mGLEnvironment) {
            auto sharedContextPtr = EGLDelegate::getInstance().getSharedContext();
            if (sharedContextPtr != 0) {
                mGLEnvironment = EGLDelegate::getInstance().createEGLEnvironment();
                mGLEnvironment->createEGLContext(sharedContextPtr);
                mGLEnvironment->createPBufferEGLSurface(4, 4);//后台渲染直接基于FrameBuffer不使用Surface
                mGLEnvironment->makeEGLCurrent();
                return true;
            }
            return false;
        }
        return true;
    }

    std::shared_ptr <FrameBuffer> VideoTrack::getEmptyFrameBuffer() {
        auto result = mRecycleQueue->pop();
        if (!result) {
            result = std::make_shared<FrameBuffer>();
            uint32_t width = 0;
            uint32_t height = 0;
            mTrackSize.getSize(width, height);
            result->init(width, height);
        }
        return result;
    }

    void VideoTrack::recycleFrameBuffer(std::shared_ptr <FrameBuffer> &&frameBuffer) {
        mRecycleQueue->push(frameBuffer);
    }

    std::shared_ptr <FrameBuffer> VideoTrack::getCurrentFrameBuffer() {
        auto frame = mDataQueue->pop();
        if (frame) {
            if (mCurrentFrameBuffer) {
                recycleFrameBuffer(std::move(mCurrentFrameBuffer));
            }
            mCurrentFrameBuffer = frame;
        }
        return mCurrentFrameBuffer;
    }

    void VideoTrack::resize(uint32_t width, uint32_t height) {
        if (mTrackSize.setSize(width, height)) {
            LOGE("VideoTrack::%s sizeChanged: %dx%d", __FUNCTION__, width, height);
            mSizeChanged = true;
        }
    }

} // face