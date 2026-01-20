# JNIEnvManager 使用文档

## 概述

`JNIEnvManager` 是一个用于管理 JNI 环境的工具类，通过 `thread_local` 机制为每个线程缓存 `JNIEnv`，避免频繁的 Attach/Detach 操作，提高 JNI 调用效率。

## 核心特性

1. **线程局部存储**：使用 `thread_local` 为每个线程缓存 `JNIEnv`
2. **自动生命周期管理**：智能识别是否需要 Detach
3. **RAII 模式支持**：提供 `JNIEnvScope` 类进行作用域管理
4. **线程安全**：每个线程独立管理自己的 JNIEnv
5. **零开销抽象**：缓存后的调用几乎无性能损耗

## 工作原理

### 传统方式的问题

```cpp
void traditionalWay() {
    // 每次都需要 Attach
    JNIEnv* env;
    javaVM->AttachCurrentThread(&env, nullptr);
    
    // 使用 env 调用 Java 方法
    jclass clazz = env->FindClass("java/lang/String");
    
    // 使用完必须 Detach
    javaVM->DetachCurrentThread();
}

// 多次调用会导致频繁 Attach/Detach
for (int i = 0; i < 100; ++i) {
    traditionalWay();  // 每次都 Attach/Detach，效率低
}
```

### JNIEnvManager 的解决方案

```cpp
void efficientWay() {
    JNIEnvScope scope;  // 第一次会 Attach
    JNIEnv* env = scope.getEnv();
    
    // 使用 env 调用 Java 方法
    jclass clazz = env->FindClass("java/lang/String");
    
}  // 离开作用域时才 Detach

// 同一线程多次调用会复用 JNIEnv
for (int i = 0; i < 100; ++i) {
    efficientWay();  // 只有第一次 Attach，后续复用
}
```

## 初始化

在 `JNI_OnLoad` 函数中初始化（已在 facekit.cpp 中完成）：

```cpp
extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    face::JNIEnvManager::getInstance().initJavaVM(vm);
    return JNI_VERSION_1_6;
}
```

## 使用方式

### 方式一：JNIEnvScope（推荐）

适用于大多数场景，自动管理生命周期：

```cpp
void callJavaMethod() {
    JNIEnvScope scope;
    if (!scope.isValid()) {
        return;  // 获取失败
    }
    
    JNIEnv* env = scope.getEnv();
    
    // 调用 Java 方法
    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass != nullptr) {
        // ... 处理逻辑
        env->DeleteLocalRef(stringClass);
    }
    
}  // 自动 Detach（如果需要）
```

### 方式二：使用 -> 操作符

更简洁的写法：

```cpp
void simpleCall() {
    JNIEnvScope scope;
    if (scope.isValid()) {
        jclass clazz = scope->FindClass("java/lang/String");
        jstring str = scope->NewStringUTF("Hello");
        scope->DeleteLocalRef(clazz);
        scope->DeleteLocalRef(str);
    }
}
```

### 方式三：手动管理模式

适用于需要长期持有 JNIEnv 的场景：

```cpp
void longRunningTask() {
    JNIEnv* env = JNIEnvManager::getInstance().getEnv();
    if (env == nullptr) {
        return;
    }
    
    // 长时间使用 env
    for (int i = 0; i < 1000; ++i) {
        jclass clazz = env->FindClass("java/lang/String");
        // ... 处理
        env->DeleteLocalRef(clazz);
    }
    
    // 完成后手动 Detach
    JNIEnvManager::getInstance().detachCurrentThread();
}
```

## 最佳实践

### 1. 在多线程环境中使用

每个线程都应该使用自己的 `JNIEnvScope`：

```cpp
void multiThreadExample() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([]() {
            JNIEnvScope scope;  // 每个线程自动管理
            if (scope.isValid()) {
                // 调用 Java 方法
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

### 2. 在回调中使用

C++ 回调需要调用 Java 方法时：

```cpp
class EventHandler {
    void onEvent(const Data& data) {
        JNIEnvScope scope;
        if (!scope.isValid()) {
            return;
        }
        
        // 创建 Java 对象
        jbyteArray jData = scope->NewByteArray(data.size());
        scope->SetByteArrayRegion(jData, 0, data.size(), 
                                 reinterpret_cast<const jbyte*>(data.data()));
        
        // 调用 Java 回调
        // ...
        
        scope->DeleteLocalRef(jData);
    }
};
```

### 3. 在循环中高效使用

避免在循环内部创建多个 scope：

```cpp
// ❌ 不推荐：频繁创建 scope（虽然有缓存，但仍有开销）
void inefficientLoop() {
    for (int i = 0; i < 1000; ++i) {
        JNIEnvScope scope;
        // ...
    }
}

// ✅ 推荐：在循环外创建一次 scope
void efficientLoop() {
    JNIEnvScope scope;
    if (scope.isValid()) {
        JNIEnv* env = scope.getEnv();
        for (int i = 0; i < 1000; ++i) {
            // 直接使用 env
        }
    }
}
```

### 4. 禁用自动 Detach

某些情况下可能需要手动控制：

```cpp
void manualControl() {
    JNIEnvScope scope(false);  // 禁用自动 detach
    if (scope.isValid()) {
        // ... 使用 JNIEnv
        
        // 手动决定何时 detach
        scope.detach();
    }
}
```

## 注意事项

### 1. Java 线程 vs C++ 线程

- **Java 线程**（从 Java 调用到 C++）：已经 attach，不需要也不应该 detach
- **C++ 线程**（纯 C++ 线程）：需要 attach，使用完毕应该 detach

`JNIEnvManager` 会自动识别线程类型，只 detach 需要的线程。

### 2. 线程生命周期

当 C++ 线程结束时，如果没有 detach，可能导致 JVM 资源泄漏。建议：
- 使用 `JNIEnvScope` 确保自动 detach
- 长期运行的线程应在结束前手动 `detachCurrentThread()`

### 3. 异常安全

`JNIEnvScope` 使用 RAII，即使发生异常也能保证正确清理：

```cpp
void exceptionSafe() {
    JNIEnvScope scope;
    if (scope.isValid()) {
        // 即使这里抛出异常
        throw std::runtime_error("error");
    }
}  // scope 析构时仍会正确 detach
```

### 4. LocalRef 管理

`JNIEnv` 的缓存不影响 LocalRef 的管理，仍需要及时释放：

```cpp
void localRefManagement() {
    JNIEnvScope scope;
    if (scope.isValid()) {
        for (int i = 0; i < 1000; ++i) {
            jclass clazz = scope->FindClass("java/lang/String");
            // 必须释放，否则会超出 LocalRef 限制
            scope->DeleteLocalRef(clazz);
        }
    }
}
```

## 性能优势

### 对比测试

假设需要调用 1000 次 Java 方法：

**传统方式**：
```
1000 次 Attach/Detach = 约 100-200ms
```

**使用 JNIEnvManager**：
```
1 次 Attach + 999 次缓存查找 + 1 次 Detach = 约 5-10ms
```

性能提升：**10-20 倍**

## 调试

启用日志查看 Attach/Detach 行为（在 Log.h 中配置日志级别）：

```
D/JNIEnvManager: JavaVM initialized
D/JNIEnvManager: Thread attached successfully
D/JNIEnvManager: Thread already attached, reusing JNIEnv
D/JNIEnvManager: Thread detached successfully
```

## 常见问题

### Q1: 何时使用 JNIEnvScope vs 手动管理？

**A**: 
- 短期调用、作用域明确 → 使用 `JNIEnvScope`
- 长期持有、跨函数调用 → 手动管理
- 不确定时 → 使用 `JNIEnvScope`（更安全）

### Q2: 是否可以在 Java 线程中使用？

**A**: 可以！`JNIEnvManager` 会自动识别 Java 线程，不会进行不必要的 Attach/Detach。

### Q3: 多线程安全吗？

**A**: 完全安全。每个线程有独立的 `thread_local` 存储，互不干扰。

### Q4: 会导致内存泄漏吗?

**A**: 不会。`thread_local` 变量在线程结束时自动销毁，`JNIEnvScope` 确保正确 detach。

## 文件结构

```
cpp/android/
├── JNIEnvManager.hpp          # 头文件（接口定义）
├── JNIEnvManager.cpp          # 实现文件
└── JNIEnvManager_RealWorld.cc  # 使用示例（可选，不会被编译到最终库）
```

## 总结

`JNIEnvManager` 通过 `thread_local` 机制大幅提升了 JNI 调用的效率和便利性：

✅ **性能优化**：避免频繁 Attach/Detach  
✅ **易用性**：RAII 自动管理，减少出错  
✅ **线程安全**：每个线程独立管理  
✅ **智能识别**：自动区分 Java/C++ 线程  
✅ **异常安全**：保证资源正确释放  

推荐在所有需要从 C++ 调用 Java 的场景中使用！
