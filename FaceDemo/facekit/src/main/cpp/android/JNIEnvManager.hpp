//
// Created by wilbert on 2026/1/19.
//

#ifndef FACEDEMO_JNIENVMANAGER_HPP
#define FACEDEMO_JNIENVMANAGER_HPP

#include <jni.h>
#include <memory>
#include <mutex>

namespace face {

/**
 * JNIEnv线程管理器
 * 
 * 功能：
 * 1. 使用thread_local为每个线程缓存JNIEnv，避免频繁Attach/Detach
 * 2. 自动管理线程的Attach/Detach生命周期
 * 3. 支持RAII风格的作用域管理和手动管理两种模式
 * 
 * 使用方式1（RAII作用域管理 - 推荐）：
 * {
 *     JNIEnvScope scope;  // 构造时获取JNIEnv，析构时根据需要Detach
 *     JNIEnv* env = scope.getEnv();
 *     // 使用env调用Java方法
 * }  // 离开作用域时自动Detach（如果是新attach的线程）
 * 
 * 使用方式2（手动管理）：
 * JNIEnv* env = JNIEnvManager::getInstance().getEnv();
 * // 使用env调用Java方法
 * // 稍后手动释放
 * JNIEnvManager::getInstance().detachCurrentThread();
 */
class JNIEnvManager {
public:
    /**
     * 获取单例实例
     */
    static JNIEnvManager& getInstance();
    JNIEnvManager(const JNIEnvManager&) = delete;
    JNIEnvManager& operator=(const JNIEnvManager&) = delete;
    /**
     * 初始化JavaVM（必须在JNI_OnLoad中调用）
     * @param vm JavaVM实例
     */
    void initJavaVM(JavaVM* vm);

    /**
     * 获取JavaVM实例
     */
    JavaVM* getJavaVM();

    /**
     * 获取当前线程的JNIEnv
     * 如果当前线程未attach，会自动attach
     * @return JNIEnv指针，如果失败返回nullptr
     */
    JNIEnv* getEnv();

    /**
     * 手动Detach当前线程
     * 注意：只有通过getEnv()自动attach的线程才需要手动detach
     * Java线程和已经attach的线程不应该调用此方法
     */
    void detachCurrentThread();

private:
    JNIEnvManager() = default;
    ~JNIEnvManager() = default;

    JavaVM* javaVM_ = nullptr;
    std::mutex mutex_;

    // 线程本地存储结构
    struct ThreadJNIEnv {
        JNIEnv* env = nullptr;
        bool needDetach = false;  // 标记是否是本管理器attach的，需要detach
    };

    // thread_local存储
    static thread_local ThreadJNIEnv threadEnv_;
};

/**
 * RAII风格的JNIEnv作用域管理器
 * 
 * 在构造时获取JNIEnv，析构时自动detach（如果需要）
 * 使用方式：
 * {
 *     JNIEnvScope scope;
 *     if (scope.isValid()) {
 *         JNIEnv* env = scope.getEnv();
 *         // 使用env
 *     }
 * }  // 自动detach
 */
class JNIEnvScope {
public:
    /**
     * 构造函数：获取当前线程的JNIEnv
     * @param autoDetach 是否在析构时自动detach，默认true
     */
    explicit JNIEnvScope(bool autoDetach = true);

    /**
     * 析构函数：如果需要，自动detach当前线程
     */
    ~JNIEnvScope();

    // 禁止拷贝
    JNIEnvScope(const JNIEnvScope&) = delete;
    JNIEnvScope& operator=(const JNIEnvScope&) = delete;

    /**
     * 获取JNIEnv指针
     */
    JNIEnv* getEnv() const { return env_; }

    /**
     * 检查JNIEnv是否有效
     */
    bool isValid() const { return env_ != nullptr; }

    /**
     * 重载->操作符，方便直接调用JNIEnv方法
     */
    JNIEnv* operator->() const { return env_; }

    /**
     * 手动提前detach（会防止析构时再次detach）
     */
    void detach();

private:
    JNIEnv* env_ = nullptr;
    bool autoDetach_ = true;
    bool detached_ = false;
};

} // namespace face

#endif // FACEDEMO_JNIENVMANAGER_HPP
