//
// Created by wilbert on 2026/1/26.
//

#include "VideoTrack.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLEnvironment.hpp"
#include "render/FrameBuffer.hpp"
#include "render/utils/OpenGLUtils.hpp"
#include "core/clip/VideoClip.hpp"
#include "common/Log.hpp"

namespace face {

    VideoTrack::VideoTrack(): Track(TrackType::Video) {
        size_t maxSize = 2;
        mDataQueue = std::make_shared<LimitQueue<FrameBuffer>>();
        mDataQueue->setLimitPolicy(LimitPolicy::WaitWhenBusy);
        mDataQueue->setMaxSize(maxSize);

        mRecycleQueue = std::make_shared<LimitQueue<FrameBuffer>>();
        mRecycleQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
        mRecycleQueue->setMaxSize(maxSize);
    }

    void VideoTrack::onRender(std::shared_ptr<face::Clip> clip, uint64_t timeStamp) {
        if (!clip) {
            LOGE("VideoTrack::%s[%d] no Clip", __FUNCTION__, getId());
            return;
        }
        auto renderTarget = mRenderTarget;
        if (renderTarget && initRenderEnv()) {
            auto width = renderTarget->getWidth();
            auto height = renderTarget->getHeight();
            auto condition = [timeStamp, clip, width, height] (const std::shared_ptr<FrameBuffer>& frameBuffer) -> std::shared_ptr<FrameBuffer> {
                auto localBuffer = frameBuffer;
                if (!localBuffer)  {
                    localBuffer = std::make_shared<FrameBuffer>();
                    localBuffer->init(width, height);
                    localBuffer->setScaleType(ScaleType::FitCenter);
                };

                LOGE("VideoTrack::%s renderTrace start render to textureId:%d, timeStamp:%llu", __FUNCTION__, localBuffer->getFboTexture()->getTextureId(), static_cast<unsigned long long>(timeStamp));
                if (clip->render(timeStamp, localBuffer)) {
                    return localBuffer;
                }
                return nullptr;
            };

            auto result = mRecycleQueue->pop(condition); // clip->render成功才会从RecycleQueue移出数据
            if (result) {
                LOGE("VideoTrack::%s[%d] renderTrace clipRendered to textureId:%d, timeStamp:%llu success", __FUNCTION__, getId(), result->getFboTexture()->getTextureId(), static_cast<unsigned long long>(timeStamp));
                OpenGLUtils::finish();
                mDataQueue->push(result);
            } else {
                LOGE("VideoTrack::%s[%d] renderTrace timeStamp:%llu failed", __FUNCTION__, getId(), static_cast<unsigned long long>(timeStamp));
            }
        } else {
            LOGE("VideoTrack::%s[%d] initRenderEnv failed", __FUNCTION__, getId());
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

    void VideoTrack::setRenderTarget(const std::shared_ptr<face::RenderTarget> &renderTarget) {
        mRenderTarget = renderTarget;
        mDataQueue->clear();
        mRecycleQueue->clear();
    }

} // face