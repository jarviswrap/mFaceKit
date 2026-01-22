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
    EGLSurfaceView::EGLSurfaceView(JNIEnv *env,
                                   jobject glsurfaceview,
                                   std::shared_ptr<EGLEnvironment> environment) {
        LOGE("EGLSurfaceView::%s", __FUNCTION__);
        mSurfaceView = env->NewGlobalRef(glsurfaceview);
        mEnvironment = std::move(environment);
        jclass cls = env->GetObjectClass(glsurfaceview);
        mSurfaceViewClass      = (jclass) env->NewGlobalRef(cls);
        mRequestRenderMethodID = env->GetMethodID(mSurfaceViewClass, "requestRender", "()V");
        env->DeleteLocalRef(cls);

        mRender = std::make_shared<PixelRender>();
        auto render = mRender;
        mEnvironment->setEGLSurfaceListener([render](const EGLEventId &eventId,
                                                     const EGLSurfaceType &surfaceType,
                                                     const Size<uint16_t> &size) -> void {
            LOGE("onEGLSurfaceChanged, %s, type:%s, %ux%u",
                 eventId == EGLEventId::SurfaceCreated ? "created" : (eventId ==
                                                                      EGLEventId::SurfaceOnDestroy
                                                                      ? "destroyed" : "unknown"),
                 surfaceType == EGLSurfaceType::Window ? "window" : (surfaceType ==
                                                                     EGLSurfaceType::PBuffer
                                                                     ? "PBuffer" : "noWindow"),
                 size.getWidth(), size.getHeight());
            if (surfaceType == EGLSurfaceType::Window) {
                if (eventId == EGLEventId::SurfaceCreated) {
                    render->onSurfaceChanged(size.getWidth(), size.getHeight());
                } else {
                    if (eventId == EGLEventId::SurfaceOnDestroy) {
                        render->onDestroy();
                    }
                }
            }
        });
    }

    EGLSurfaceView::~EGLSurfaceView() {
        LOGE("EGLSurfaceView::%s", __FUNCTION__);
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

    void EGLSurfaceView::setScaleType(int scaleType) {
        if (mRender) {
            std::dynamic_pointer_cast<PixelRender>(mRender)->setScaleType(static_cast<ScaleType>(scaleType));
        }
    }

    Error EGLSurfaceView::onRequestConsume(uint32_t requestId) {
        if (requestId == 1) {
            LOGE("EGLSurfaceView::%s firstRequest", __FUNCTION__);
        }
        auto env = JNIEnvManager::getInstance().getEnv();
        if (env && mSurfaceView && mRequestRenderMethodID) {
            env->CallVoidMethod(mSurfaceView, mRequestRenderMethodID);
            return Error::None;
        }
        return Error::Err_InvalidJniMethod;
    }

    Error EGLSurfaceView::onConsumeData(const std::shared_ptr<PixelData> &data) {
        if (!mCurrentData) {
            LOGE("EGLSurfaceView::%s firstDraw", __FUNCTION__);
        }
        if (data) { // 空数据时重复渲染上帧（避免两个问题：1. 多余requestRender时引入黑帧闪烁问题，2. GLSurfaceView首个onDrawFrame无法正常渲染）
            mCurrentData = data;
        }
        auto res = mRender->onDrawFrame(mCurrentData);
        return res;
    }

    void EGLSurfaceView::onDestroy() {
        LOGE("EGLSurfaceView::%s", __FUNCTION__);
        JNIEnvManager::getInstance().detachCurrentThread();
    }
} // face