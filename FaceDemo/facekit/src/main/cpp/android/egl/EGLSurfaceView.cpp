//
// Created by wilbert on 2026/1/20.
//

#include "EGLSurfaceView.hpp"

namespace face {
    EGLSurfaceView::EGLSurfaceView(JNIEnv *env, jobject glsurfaceview) {
        mSurfaceView = env->NewGlobalRef(glsurfaceview);
    }

    EGLSurfaceView::~EGLSurfaceView() {

    }

    Error EGLSurfaceView::onRequestConsume(uint32_t requestId) {
        return Error::Err_ModelInvalid;
    }

    Error EGLSurfaceView::onConsumeData(PixelData data) {
        return Error::Err_ModelInvalid;
    }

    void EGLSurfaceView::release(JNIEnv *env) {
        if (mSurfaceView) {
            env->DeleteGlobalRef(mSurfaceView);
            mSurfaceView = nullptr;
        }
    }
} // face