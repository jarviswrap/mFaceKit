//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLENVIRONMENT_HPP
#define FACEDEMO_EGLENVIRONMENT_HPP
#include <vector>
#include <EGL/egl.h>

namespace face {

    enum class Scene: uint8_t {
        BACKGROUND_RENDER = 1,
        DISPLAY = 2,
        HARDWARE_ENCODE = 4,
    };

    class EGLEnvironment {
    public:
        EGLEnvironment(bool tryGLES3 = false);
        ~EGLEnvironment();

        int64_t createEGLContext(int64_t shared_context_ptr = 0, uint8_t scene = (uint8_t)Scene::BACKGROUND_RENDER);
        bool destroyEGLContext();

        int64_t createWindowEGLSurface(void* aNativeWindow);
        int64_t createPBufferEGLSurface(int32_t width, int32_t height);
        bool destroyEGLSurface();

        bool makeEGLCurrent();

        int32_t getEGLSurfaceWidth();
        int32_t getEGLSurfaceHeight();

        int64_t getEGLContext();
        int64_t getEGLSurface();
        int64_t getSharedFromEGLContext();
    private:
        bool mTryGLES3{false};
        EGLDisplay mDisplay{EGL_NO_DISPLAY};
        EGLConfig mConfig{nullptr};
        EGLContext mContext{EGL_NO_CONTEXT};
        EGLSurface mSurface{EGL_NO_SURFACE};
        int64_t mSharedContextPtr{0};
    };

} // face

#endif //FACEDEMO_EGLENVIRONMENT_HPP
