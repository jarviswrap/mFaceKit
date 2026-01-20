//
// Created by wilbert on 2026/1/20.
//

#include "EGLDelegate.hpp"

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

    int64_t EGLDelegate::createEGLSurfaceView(JNIEnv *env, jobject eglSurfaceView) {
        return 0;
    }

} // face
