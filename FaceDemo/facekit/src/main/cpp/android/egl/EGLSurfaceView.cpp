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
                                   jobject glsurfaceview) {
        LOGE("EGLSurfaceView::%s", __FUNCTION__);
        mSurfaceView = env->NewGlobalRef(glsurfaceview);
        jclass cls = env->GetObjectClass(glsurfaceview);
        mSurfaceViewClass      = (jclass) env->NewGlobalRef(cls);
        mRequestRenderMethodID = env->GetMethodID(mSurfaceViewClass, "requestRender", "()V");
        env->DeleteLocalRef(cls);
    }

    void EGLSurfaceView::initEnvironment(std::shared_ptr <face::EGLEnvironment> environment) {
        mEnvironment = std::move(environment);
        mRender = std::make_shared<PixelRender>();
        auto render = mRender;
        std::weak_ptr<EGLSurfaceView> weakPtr(shared_from_this());
        mEnvironment->setEGLSurfaceListener([weakPtr](const EGLEventId &eventId,
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
            if (auto ptr = weakPtr.lock()) {
                if (surfaceType == EGLSurfaceType::Window) {
                    if (eventId == EGLEventId::SurfaceCreated) {
                        ptr->onSurfaceChanged(size.getWidth(), size.getHeight());
                    } else if (eventId == EGLEventId::SurfaceOnDestroy) {
                        ptr->onSurfaceDestroy();
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

    void EGLSurfaceView::setSurfaceListener(DataListener<uint32_t, uint32_t> listener) {
        mSurfaceListener = listener;
        auto environment = mEnvironment;
        if (listener && environment && environment->isReady() && environment->getSurfaceType() == EGLSurfaceType::Window) { // 如果已经初始化好了EGL环境，则会在设置回调时默认回调一次
            listener(environment->getEGLSurfaceWidth(), environment->getEGLSurfaceHeight());
        }
    }

    Error EGLSurfaceView::requestDraw() {
        if (mRequestIndex == 0) {
            LOGE("EGLSurfaceView::%s firstRequest", __FUNCTION__);
        }
        auto env = JNIEnvManager::getInstance().getEnv();
        if (env && mSurfaceView && mRequestRenderMethodID) {
            env->CallVoidMethod(mSurfaceView, mRequestRenderMethodID);
            mRequestIndex++;
            return Error::None;
        }
        return Error::Err_InvalidJniMethod;
    }

    Error EGLSurfaceView::destroy() {
        LOGE("EGLSurfaceView::%s", __FUNCTION__);
        JNIEnvManager::getInstance().detachCurrentThread();
        return Error::None;
    }


    void EGLSurfaceView::onDraw() {
        if (mDrawIndex ==  0) {
            LOGE("EGLSurfaceView::%s firstDraw, requestIndex:%d", __FUNCTION__, mRequestIndex.load());
        }
        auto drawListener = mDrawListener;
        if (drawListener) drawListener();
        mDrawIndex++;
    }

    void EGLSurfaceView::onSurfaceChanged(uint32_t width, uint32_t height) {
        auto surfaceListener = mSurfaceListener;
        if (surfaceListener) surfaceListener(width, height);
    }

    void EGLSurfaceView::onSurfaceDestroy() {
        auto surfaceDestroyListener = mSurfaceDestroyListener;
        if (surfaceDestroyListener) surfaceDestroyListener();
    }

    void EGLSurfaceView::getSurfaceSize(uint32_t &width, uint32_t &height) {
        mSurfaceSize.getSize(width, height);
    }
} // face