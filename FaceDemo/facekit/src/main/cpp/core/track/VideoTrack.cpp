//
// Created by wilbert on 2026/1/26.
//

#include "VideoTrack.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLEnvironment.hpp"
#include "render/utils/FrameBuffer.hpp"
#include "core/clip/VideoClip.hpp"
#include "common/Log.hpp"

namespace face {


    void VideoTrack::onRender(std::shared_ptr<face::Clip> clip, uint64_t timeStamp) {
        if (initRenderEnv()) {
            if (mSizeChanged) {
                mFrameBufferList.clear();
                uint32_t width = 0;
                uint32_t height = 0;
                mTrackSize.getSize(width, height);
                for (int i = 0; i< FRAME_BUFFER_SIZE; i++) {
                    auto frameBuffer = std::make_shared<FrameBuffer>();
                    frameBuffer->init(width, height);
                    mFrameBufferList.push_back(frameBuffer);
                }
                for(auto& c: mClips) {
                    if (clip->getClipType() == ClipType::Video) {
                        auto videoClip = std::static_pointer_cast<VideoClip>(c);
                        videoClip->onSizeChanged(width, height);
                    }
                }
                mSizeChanged = false;
            }
            auto frameBufferIndex = mFrameBufferId % mFrameBufferList.size();
            auto frameBuffer = mFrameBufferList[mFrameBufferId++];
            frameBuffer->bind();
            clip->render(timeStamp);
            mCurrentTexture = frameBuffer->getFboTexture();
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

    void VideoTrack::resize(uint32_t width, uint32_t height) {
        if (mTrackSize.setSize(width, height)) {
            LOGE("VideoTrack::%s sizeChanged: %dx%d", __FUNCTION__, width, height);
            mSizeChanged = true;
        }
    }

} // face