//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLENVIRONMENT_HPP
#define FACEDEMO_EGLENVIRONMENT_HPP
#include <utility>
#include <vector>
#include <EGL/egl.h>
#include "common/Common.hpp"
#include "common/Size.hpp"

namespace face {

    enum class Scene: uint8_t {
        BACKGROUND_RENDER = 1,
        DISPLAY = 2,
        HARDWARE_ENCODE = 4,
    };

    enum class EGLEventId: uint8_t {
        SurfaceCreated, SurfaceOnDestroy
    };

    enum class EGLSurfaceType: uint8_t {
        NoSurface, PBuffer, Window
    };

    class EGLEnvironment {
    public:
        explicit EGLEnvironment(bool tryGLES3 = false);
        ~EGLEnvironment();

        int64_t createEGLContext(int64_t shared_context_ptr = 0, uint8_t scene = (uint8_t)Scene::BACKGROUND_RENDER);
        bool destroyEGLContext(); // 要求destroyEGLContext的线程环境和makeEGLCurrent线程环境一致

        int64_t createWindowEGLSurface(void* aNativeWindow);
        int64_t createPBufferEGLSurface(int32_t width, int32_t height);
        bool destroyEGLSurface(); // 要求destroyEGLSurface的线程环境和makeEGLCurrent线程环境一致

        bool makeEGLCurrent();

        int32_t getEGLSurfaceWidth();
        int32_t getEGLSurfaceHeight();

        int64_t getEGLContext();
        int64_t getEGLSurface();
        int64_t getSharedFromEGLContext() const;

        bool isReady() const;

        void setEGLSurfaceListener(DataListener<EGLEventId, EGLSurfaceType, Size<uint16_t>> surfaceListener) { mSurfaceListener = std::move(surfaceListener); };
    private:
        bool mTryGLES3{false};
        EGLDisplay mDisplay{EGL_NO_DISPLAY};
        EGLConfig mConfig{nullptr};
        EGLContext mContext{EGL_NO_CONTEXT};
        EGLSurface mSurface{EGL_NO_SURFACE};
        EGLSurfaceType mSurfaceType{EGLSurfaceType::NoSurface};
        int64_t mSharedContextPtr{0};
        Size<uint16_t> mSurfaceResolution{0, 0};
        DataListener<EGLEventId, EGLSurfaceType, Size<uint16_t>> mSurfaceListener{nullptr};
    };

} // face

#endif //FACEDEMO_EGLENVIRONMENT_HPP
