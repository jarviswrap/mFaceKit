//
// Created by wilbert on 2026/1/20.
//

#include "EGLDelegate.hpp"
#include "EGLEnvironment.hpp"
#include "EGLSurfaceView.hpp"
#include "common/Log.hpp"

namespace face {

    EGLDelegate& EGLDelegate::getInstance() {
        static EGLDelegate instance;
        return instance;
    }

    int64_t EGLDelegate::createEGLEnvironment() {
        std::lock_guard<std::mutex> lock(mMutex);
        auto environment = std::make_shared<EGLEnvironment>();
        auto ptr = (int64_t) environment.get();
        mEnvironments.emplace(ptr, environment);
        return ptr;
    }

    std::shared_ptr<EGLEnvironment> EGLDelegate::getEGLEnvironment(int64_t environment_ptr) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mEnvironments.find(environment_ptr);
        if (it != mEnvironments.end()) {
            return it->second;
        }
        return nullptr;
    }

    void EGLDelegate::removeEGLEnvironment(int64_t environment_ptr) {
        std::lock_guard<std::mutex> lock(mMutex);
        mEnvironments.erase(environment_ptr);
    }

    int64_t EGLDelegate::getSharedContext() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mEnvironments.empty()) {
            return 0;
        }
        for (const auto& pair : mEnvironments) {
            auto env = pair.second;
            int64_t sharedFrom = env->getSharedFromEGLContext();
            if (sharedFrom != 0) {
                return sharedFrom;
            } else {
                return env->getEGLContext();
            }
        }
        return 0;
    }

    int64_t EGLDelegate::createEGLSurfaceView(JNIEnv *env, jobject eglSurfaceView, int64_t eglEnvironmentPtr) {
        std::lock_guard<std::mutex> lock(mMutex);
        std::shared_ptr<EGLEnvironment> eglEnvironment;
        auto it = mEnvironments.find(eglEnvironmentPtr);
        if (it != mEnvironments.end()) {
            eglEnvironment = it->second;
        }
        auto showView = std::make_shared<EGLSurfaceView>(env, eglSurfaceView, eglEnvironment);
        auto ptr = (int64_t) showView.get();
        mSurfaceViews.emplace(ptr, showView);
        mLastShowView = showView;
        return ptr;
    }

    std::shared_ptr<EGLSurfaceView> EGLDelegate::getShowView(int64_t surfaceview_ptr) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (surfaceview_ptr == 0 && mLastShowView) {
            return mLastShowView;
        }
        auto it = mSurfaceViews.find(surfaceview_ptr);
        if (it != mSurfaceViews.end()) {
            return it->second;
        }
        return nullptr;
    }

    void EGLDelegate::removeEGLShowView(int64_t surfaceview_ptr) {
        std::lock_guard<std::mutex> lock(mMutex);
        mSurfaceViews.erase(surfaceview_ptr);
        if (mLastShowView) {
            auto ptr = (int64_t) mLastShowView.get();
            if (surfaceview_ptr == ptr) {
                mLastShowView.reset();
            }
        }
    }

    std::shared_ptr<ImagePreviewer> EGLDelegate::getImagePreviewer() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mImagePreviewer) {
            mImagePreviewer = std::shared_ptr<ImagePreviewer>();
        }
        return mImagePreviewer;
    }


} // face
