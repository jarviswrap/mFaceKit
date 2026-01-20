//
// Created by wilbert on 2026/1/19.
//

#include "JNIEnvManager.hpp"
#include "common/Log.hpp"

namespace face {

// thread_local变量定义
thread_local JNIEnvManager::ThreadJNIEnv JNIEnvManager::threadEnv_;

JNIEnvManager& JNIEnvManager::getInstance() {
    static JNIEnvManager instance;
    return instance;
}

void JNIEnvManager::initJavaVM(JavaVM* vm) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (javaVM_ == nullptr) {
        javaVM_ = vm;
        LOGI("JNIEnvManager: JavaVM initialized");
    } else if (javaVM_ != vm) {
        LOGE("JNIEnvManager: JavaVM already initialized with different instance");
    }
}

JavaVM* JNIEnvManager::getJavaVM() {
    return javaVM_;
}

JNIEnv* JNIEnvManager::getEnv() {
    if (javaVM_ == nullptr) {
        LOGE("JNIEnvManager: JavaVM not initialized, call initJavaVM first");
        return nullptr;
    }

    // 检查thread_local缓存
    if (threadEnv_.env != nullptr) {
        return threadEnv_.env;
    }

    JNIEnv* env = nullptr;
    
    // 尝试获取当前线程的JNIEnv（如果已经attach）
    jint result = javaVM_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    
    if (result == JNI_OK) {
        // 当前线程已经attach（可能是Java线程或之前已attach的native线程）
        threadEnv_.env = env;
        threadEnv_.needDetach = false;
        LOGI("JNIEnvManager: Thread already attached, reusing JNIEnv");
        return env;
    }
    
    if (result == JNI_EDETACHED) {
        // 当前线程未attach，需要attach
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_6;
        args.name = nullptr;  // 可以设置线程名称
        args.group = nullptr;
        
        result = javaVM_->AttachCurrentThread(&env, &args);
        
        if (result == JNI_OK && env != nullptr) {
            threadEnv_.env = env;
            threadEnv_.needDetach = true;  // 标记需要detach
            LOGI("JNIEnvManager: Thread attached successfully");
            return env;
        } else {
            LOGE("JNIEnvManager: Failed to attach thread, error code: %d", result);
            return nullptr;
        }
    }
    
    LOGE("JNIEnvManager: GetEnv failed with error code: %d", result);
    return nullptr;
}

void JNIEnvManager::detachCurrentThread() {
    if (javaVM_ == nullptr) {
        LOGE("JNIEnvManager: JavaVM not initialized");
        return;
    }

    // 只detach那些通过本管理器attach的线程
    if (threadEnv_.needDetach && threadEnv_.env != nullptr) {
        javaVM_->DetachCurrentThread();
        threadEnv_.env = nullptr;
        threadEnv_.needDetach = false;
        LOGI("JNIEnvManager: Thread detached successfully");
    } else {
        LOGI("JNIEnvManager: Thread doesn't need detach (either not attached by manager or Java thread)");
    }
}

// ============ JNIEnvScope实现 ============

JNIEnvScope::JNIEnvScope(bool autoDetach) 
    : autoDetach_(autoDetach), detached_(false) {
    env_ = JNIEnvManager::getInstance().getEnv();
}

JNIEnvScope::~JNIEnvScope() {
    if (autoDetach_ && !detached_) {
        detach();
    }
}

void JNIEnvScope::detach() {
    if (!detached_) {
        JNIEnvManager::getInstance().detachCurrentThread();
        detached_ = true;
    }
}

} // namespace face

