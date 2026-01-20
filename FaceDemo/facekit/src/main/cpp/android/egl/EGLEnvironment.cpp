//
// Created by wilbert on 2026/1/20.
//

#include "EGLEnvironment.hpp"
#include "common/Log.hpp"
#include <EGL/eglext.h>


#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040
#endif

#ifndef EGL_RECORDABLE_ANDROID
#define EGL_RECORDABLE_ANDROID 0x3142
#endif

namespace face {

    EGLEnvironment::EGLEnvironment(bool tryGLES3): mTryGLES3(tryGLES3) {
    }

    EGLEnvironment::~EGLEnvironment() {
        destroyEGLContext();
    }

    int64_t EGLEnvironment::createEGLContext(int64_t shared_context_ptr, uint8_t scene) {
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (mDisplay == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay failed");
            return 0;
        }

        EGLint major, minor;
        if (!eglInitialize(mDisplay, &major, &minor)) {
            LOGE("eglInitialize failed");
            return 0;
        }

        auto isScene = [] (uint8_t sc, Scene compare) -> bool {
            return (sc & (uint8_t) compare);
        };
        int renderableNum = 0;
        auto renderable_type = 0;
        if (isScene(scene, Scene::DISPLAY)|| isScene(scene, Scene::HARDWARE_ENCODE)) {
            renderable_type |= EGL_WINDOW_BIT;
            renderableNum++;
        }
        if (isScene(scene, Scene::BACKGROUND_RENDER)) {
            renderable_type |= EGL_PBUFFER_BIT;
            renderableNum++;
        }
        std::vector<EGLint> configAttribs;
        configAttribs.push_back(EGL_RENDERABLE_TYPE);
        configAttribs.push_back(mTryGLES3 ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT);
        configAttribs.push_back(EGL_SURFACE_TYPE);
        configAttribs.push_back(renderable_type);
        configAttribs.push_back(EGL_BLUE_SIZE);
        configAttribs.push_back(8);
        configAttribs.push_back(EGL_GREEN_SIZE);
        configAttribs.push_back(8);
        configAttribs.push_back(EGL_RED_SIZE);
        configAttribs.push_back(8);
        configAttribs.push_back(EGL_ALPHA_SIZE);
        configAttribs.push_back(8);
        configAttribs.push_back(EGL_DEPTH_SIZE);
        configAttribs.push_back(16);
        if (isScene(scene, Scene::HARDWARE_ENCODE)) {
            configAttribs.push_back(EGL_RECORDABLE_ANDROID);
            configAttribs.push_back(1);
        }
        configAttribs.push_back(EGL_NONE);

        int numConfigs;
        if (!eglChooseConfig(mDisplay, configAttribs.data(), &mConfig, 1, &numConfigs) || numConfigs == 0) {
            // Fallback to ES2 if ES3 failed or no config found
            if (mTryGLES3) {
                // Find EGL_RENDERABLE_TYPE and update it
                for (size_t i = 0; i < configAttribs.size(); ++i) {
                    if (configAttribs[i] == EGL_RENDERABLE_TYPE) {
                        configAttribs[i + 1] = EGL_OPENGL_ES2_BIT;
                        break;
                    }
                }
                if (!eglChooseConfig(mDisplay, configAttribs.data(), &mConfig, 1, &numConfigs) || numConfigs == 0) {
                    LOGE("eglChooseConfig failed, renderableNum:%d, renderable_type:%d", renderableNum, renderable_type);
                    return 0;
                }
            } else {
                LOGE("eglChooseConfig failed, renderableNum:%d, renderable_type:%d", renderableNum, renderable_type);
                return 0;
            }
        }

        EGLint contextAttribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, mTryGLES3 ? 3 : 2,
                EGL_NONE
        };

        EGLContext sharedContext = (shared_context_ptr != 0) ? (EGLContext)shared_context_ptr : EGL_NO_CONTEXT;
        mContext = eglCreateContext(mDisplay, mConfig, sharedContext, contextAttribs);

        if (mContext == EGL_NO_CONTEXT && mTryGLES3) {
            // Retry with ES2
            contextAttribs[1] = 2;
            mContext = eglCreateContext(mDisplay, mConfig, sharedContext, contextAttribs);
        }

        if (mContext == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed, renderableNum:%d, renderable_type:%d", renderableNum, renderable_type);
            return 0;
        }

        mSharedContextPtr = shared_context_ptr;

        return (int64_t)mContext;
    }

    bool EGLEnvironment::destroyEGLContext() {
        if (mDisplay != EGL_NO_DISPLAY) {
            destroyEGLSurface();
            if (mContext != EGL_NO_CONTEXT) {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }
            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
            return true;
        }
        return false;
    }

    int64_t EGLEnvironment::createWindowEGLSurface(void *aNativeWindow) {
        if (mDisplay == EGL_NO_DISPLAY || mConfig == nullptr) {
            return 0;
        }
        // destroy existing surface if any
        if (mSurface != EGL_NO_SURFACE) {
            destroyEGLSurface();
        }

        mSurface = eglCreateWindowSurface(mDisplay, mConfig, (EGLNativeWindowType)aNativeWindow, nullptr);
        if (mSurface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed");
            return 0;
        }
        return (int64_t)mSurface;
    }

    int64_t EGLEnvironment::createPBufferEGLSurface(int32_t width, int32_t height) {
        if (mDisplay == EGL_NO_DISPLAY || mConfig == nullptr) {
            return 0;
        }
        if (mSurface != EGL_NO_SURFACE) {
            destroyEGLSurface();
        }

        EGLint attribs[] = {
                EGL_WIDTH, width,
                EGL_HEIGHT, height,
                EGL_NONE
        };
        mSurface = eglCreatePbufferSurface(mDisplay, mConfig, attribs);
        if (mSurface == EGL_NO_SURFACE) {
            LOGE("eglCreatePbufferSurface failed");
            return 0;
        }
        return (int64_t)mSurface;
    }

    bool EGLEnvironment::destroyEGLSurface() {
        if (mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
            mSurface = EGL_NO_SURFACE;
            return true;
        }
        return false;
    }

    bool EGLEnvironment::makeEGLCurrent() {
        if (mDisplay != EGL_NO_DISPLAY && mContext != EGL_NO_CONTEXT && mSurface != EGL_NO_SURFACE) {
            if (!eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)) {
                LOGE("eglMakeCurrent failed");
                return false;
            }
            return true;
        }
        return false;
    }

    int32_t EGLEnvironment::getEGLSurfaceWidth() {
        if (mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE) {
            EGLint width;
            eglQuerySurface(mDisplay, mSurface, EGL_WIDTH, &width);
            return width;
        }
        return 0;
    }

    int32_t EGLEnvironment::getEGLSurfaceHeight() {
        if (mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE) {
            EGLint height;
            eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &height);
            return height;
        }
        return 0;
    }

    int64_t EGLEnvironment::getEGLContext() {
        return (int64_t)mContext;
    }

    int64_t EGLEnvironment::getEGLSurface() {
        return (int64_t)mSurface;
    }

    int64_t EGLEnvironment::getSharedFromEGLContext() {
        return mSharedContextPtr;
    }

} // face
