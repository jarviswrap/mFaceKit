//
// Created by wilbert on 2026/1/19.
//
// 实际应用示例：在现有项目中集成 JNIEnvManager

#ifdef __ANDROID__

#include "JNIEnvManager.hpp"
#include "AndroidUtils.hpp"
#include <thread>
#include <functional>

namespace face {

/**
 * 示例：改造前后的对比
 */
class RealWorldExample {
public:
    
    // ========== 改造前：传统方式 ==========
    
    /**
     * 传统方式：在 C++ 线程中调用 Java 方法
     * 问题：每次调用都需要 Attach/Detach
     */
    void oldWay_CallJavaFromThread(JavaVM* vm) {
        std::thread worker([vm]() {
            JNIEnv* env = nullptr;
            
            // 每次都要 Attach
            if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                return;
            }
            
            // 调用 Java 方法
            jclass clazz = env->FindClass("com/jarvis/facekit/FaceKit");
            if (clazz != nullptr) {
                // 处理逻辑
                env->DeleteLocalRef(clazz);
            }
            
            // 必须记得 Detach
            vm->DetachCurrentThread();
        });
        worker.join();
    }
    
    /**
     * 传统方式：在回调中调用 Java（问题更严重）
     */
    class OldCallbackHandler {
    private:
        JavaVM* vm_;
        
    public:
        OldCallbackHandler(JavaVM* vm) : vm_(vm) {}
        
        void onDataReceived(const std::vector<uint8_t>& data) {
            JNIEnv* env = nullptr;
            // 每次回调都 Attach - 性能问题！
            if (vm_->AttachCurrentThread(&env, nullptr) != JNI_OK) {
                return;
            }
            
            // 创建 Java byte 数组
            jbyteArray jData = env->NewByteArray(data.size());
            env->SetByteArrayRegion(jData, 0, data.size(),
                                   reinterpret_cast<const jbyte*>(data.data()));
            
            // 调用 Java 回调
            // ... 
            
            env->DeleteLocalRef(jData);
            
            // 每次都 Detach - 性能问题！
            vm_->DetachCurrentThread();
        }
    };
    
    // ========== 改造后：使用 JNIEnvManager ==========
    
    /**
     * 新方式：使用 JNIEnvScope
     * 优势：自动管理，代码更简洁，性能更好
     */
    void newWay_CallJavaFromThread() {
        std::thread worker([]() {
            JNIEnvScope scope;  // 自动 Attach
            if (!scope.isValid()) {
                return;
            }
            
            // 直接使用，代码更简洁
            jclass clazz = scope->FindClass("com/jarvis/facekit/FaceKit");
            if (clazz != nullptr) {
                // 处理逻辑
                scope->DeleteLocalRef(clazz);
            }
            
            // 自动 Detach，无需手动管理
        });
        worker.join();
    }
    
    /**
     * 新方式：回调处理器（性能优化）
     */
    class NewCallbackHandler {
    public:
        void onDataReceived(const std::vector<uint8_t>& data) {
            JNIEnvScope scope;  // thread_local 缓存，只第一次 Attach
            if (!scope.isValid()) {
                return;
            }
            
            // 创建 Java byte 数组
            jbyteArray jData = scope->NewByteArray(data.size());
            scope->SetByteArrayRegion(jData, 0, data.size(),
                                     reinterpret_cast<const jbyte*>(data.data()));
            
            // 调用 Java 回调
            // ...
            
            scope->DeleteLocalRef(jData);
            // 自动管理，无需担心忘记 Detach
        }
    };
};

/**
 * 示例：与 AndroidUtils 配合使用
 * 展示如何在实际业务代码中使用
 */
class AndroidUtilsIntegration {
public:
    
    /**
     * 示例：从 C++ 线程调用 Java 方法并传递数据
     */
    static void sendDataToJava(const std::vector<int32_t>& data) {
        JNIEnvScope scope;
        if (!scope.isValid()) {
            return;
        }
        
        JNIEnv* env = scope.getEnv();
        
        // 使用 AndroidUtils 创建 Java 数组
        jintArray jData = AndroidUtils::createIntArray(env, 
                            const_cast<std::vector<int32_t>&>(data));
        
        // 查找 Java 类和方法
        jclass clazz = env->FindClass("com/jarvis/facekit/DataReceiver");
        if (clazz != nullptr) {
            jmethodID method = env->GetStaticMethodID(clazz, 
                                                     "onDataReceived", 
                                                     "([I)V");
            if (method != nullptr) {
                env->CallStaticVoidMethod(clazz, method, jData);
            }
            env->DeleteLocalRef(clazz);
        }
        
        if (jData != nullptr) {
            env->DeleteLocalRef(jData);
        }
    }
    
    /**
     * 示例：从 C++ 获取 Java 字符串
     */
    static std::string getJavaString() {
        JNIEnvScope scope;
        if (!scope.isValid()) {
            return "";
        }
        
        JNIEnv* env = scope.getEnv();
        
        // 调用 Java 方法获取字符串
        jclass clazz = env->FindClass("com/jarvis/facekit/ConfigProvider");
        if (clazz == nullptr) {
            return "";
        }
        
        jmethodID method = env->GetStaticMethodID(clazz, 
                                                 "getConfig", 
                                                 "()Ljava/lang/String;");
        if (method == nullptr) {
            env->DeleteLocalRef(clazz);
            return "";
        }
        
        jstring jStr = (jstring)env->CallStaticObjectMethod(clazz, method);
        
        // 使用 AndroidUtils 转换
        std::string result = AndroidUtils::readStringUTF(env, jStr);
        
        if (jStr != nullptr) {
            env->DeleteLocalRef(jStr);
        }
        env->DeleteLocalRef(clazz);
        
        return result;
    }
};

/**
 * 示例：在图像处理线程中使用
 * 模拟一个实际的图像处理场景
 */
class ImageProcessorWithJava {
private:
    std::thread processingThread_;
    bool running_ = false;
    
public:
    void start() {
        running_ = true;
        processingThread_ = std::thread([this]() {
            // 线程开始时获取 JNIEnv（只 Attach 一次）
            JNIEnvScope scope;
            if (!scope.isValid()) {
                return;
            }
            
            JNIEnv* env = scope.getEnv();
            
            // 查找 Java 回调类（只查找一次）
            jclass callbackClass = env->FindClass("com/jarvis/facekit/ImageCallback");
            if (callbackClass == nullptr) {
                return;
            }
            
            jmethodID onFrameMethod = env->GetStaticMethodID(
                callbackClass, 
                "onFrameProcessed", 
                "([B)V"
            );
            
            if (onFrameMethod == nullptr) {
                env->DeleteLocalRef(callbackClass);
                return;
            }
            
            // 处理循环
            while (running_) {
                // 模拟处理图像
                std::vector<uint8_t> imageData = processImage();
                
                // 创建 Java byte 数组
                jbyteArray jData = env->NewByteArray(imageData.size());
                env->SetByteArrayRegion(jData, 0, imageData.size(),
                                       reinterpret_cast<const jbyte*>(imageData.data()));
                
                // 调用 Java 回调
                env->CallStaticVoidMethod(callbackClass, onFrameMethod, jData);
                
                // 释放本地引用
                env->DeleteLocalRef(jData);
                
                // 避免 LocalRef 累积过多
                if (env->PushLocalFrame(256) == 0) {
                    // 处理更多逻辑
                    env->PopLocalFrame(nullptr);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
            
            env->DeleteLocalRef(callbackClass);
            // scope 析构时自动 Detach
        });
    }
    
    void stop() {
        running_ = false;
        if (processingThread_.joinable()) {
            processingThread_.join();
        }
    }
    
private:
    std::vector<uint8_t> processImage() {
        // 模拟图像处理
        return std::vector<uint8_t>(1920 * 1080 * 4);
    }
};

/**
 * 示例：线程池中使用 JNIEnv
 */
class ThreadPoolWithJNI {
private:
    std::vector<std::thread> workers_;
    
public:
    void initialize(int threadCount) {
        for (int i = 0; i < threadCount; ++i) {
            workers_.emplace_back([this, i]() {
                // 每个工作线程获取自己的 JNIEnv
                JNIEnvScope scope;
                if (!scope.isValid()) {
                    return;
                }
                
                JNIEnv* env = scope.getEnv();
                
                // 查找需要的 Java 类（每个线程一次）
                jclass logClass = env->FindClass("android/util/Log");
                jmethodID logMethod = nullptr;
                if (logClass != nullptr) {
                    logMethod = env->GetStaticMethodID(
                        logClass, "d",
                        "(Ljava/lang/String;Ljava/lang/String;)I"
                    );
                }
                
                // 工作循环
                while (true) {
                    // 从队列获取任务
                    auto task = getTask();
                    if (!task) break;
                    
                    // 执行任务
                    task();
                    
                    // 记录日志到 Java
                    if (logMethod != nullptr) {
                        jstring tag = env->NewStringUTF("ThreadPool");
                        jstring msg = env->NewStringUTF("Task completed");
                        env->CallStaticIntMethod(logClass, logMethod, tag, msg);
                        env->DeleteLocalRef(tag);
                        env->DeleteLocalRef(msg);
                    }
                }
                
                if (logClass != nullptr) {
                    env->DeleteLocalRef(logClass);
                }
                // scope 析构，自动 Detach
            });
        }
    }
    
    void shutdown() {
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
private:
    std::function<void()> getTask() {
        // 模拟获取任务
        static int count = 0;
        if (++count > 10) return nullptr;
        return []() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
    }
};

} // namespace face

#endif // __ANDROID__
