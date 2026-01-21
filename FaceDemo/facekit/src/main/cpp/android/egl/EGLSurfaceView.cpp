//
// Created by wilbert on 2026/1/20.
//

#include "EGLSurfaceView.hpp"
#include "android/JNIEnvManager.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "common/Log.hpp"
#include "EGLEnvironment.hpp"
#include "render/PixelRender.hpp"

namespace face {
    EGLSurfaceView::EGLSurfaceView(JNIEnv *env, jobject glsurfaceview, std::shared_ptr<EGLEnvironment> environment) {
        mSurfaceView = env->NewGlobalRef(glsurfaceview);
        mEnvironment = std::move(environment);
        jclass cls = env->GetObjectClass(glsurfaceview);
        mSurfaceViewClass = (jclass) env->NewGlobalRef(cls);
        mRequestRenderMethodID = env->GetMethodID(mSurfaceViewClass, "requestRender", "()V");
        env->DeleteLocalRef(cls);

        mRender = std::make_shared<PixelRender>();
        auto render = mRender;
        mEnvironment->setEGLSurfaceListener([render] (const EGLEventId& eventId, const EGLSurfaceType& surfaceType, const Size<uint16_t>& size) -> void {
            if (surfaceType == EGLSurfaceType::Window) {
                if (eventId == EGLEventId::SurfaceCreated) {
                    render->onSurfaceChanged(size.getWidth(), size.getHeight());
                } else if (eventId == EGLEventId::SurfaceOnDestroy) {
                    render->onDestroy();
                }
            }
        });
    }

    EGLSurfaceView::~EGLSurfaceView() {
        if (mSurfaceView) {
            JNIEnvScope scope;
            if (scope.isValid()) {
                scope->DeleteGlobalRef(mSurfaceView);
                mSurfaceView = nullptr;
                if (mSurfaceViewClass) {
                    scope->DeleteGlobalRef(mSurfaceViewClass);
                    mSurfaceViewClass = nullptr;
                }
            }
        }
    }

    Error EGLSurfaceView::onRequestConsume(uint32_t requestId) {
        auto env = JNIEnvManager::getInstance().getEnv();
        if (env && mSurfaceView && mRequestRenderMethodID) {
            env->CallVoidMethod(mSurfaceView, mRequestRenderMethodID);
            return Error::None;
        }
        return Error::Err_InvalidJniMethod;
    }

    Error EGLSurfaceView::onConsumeData(const std::shared_ptr<PixelData>& data) {
        return Error::Err_ModelInvalid;
    }

    void EGLSurfaceView::onDestroy() {
        JNIEnvManager::getInstance().detachCurrentThread();
    }
} // face