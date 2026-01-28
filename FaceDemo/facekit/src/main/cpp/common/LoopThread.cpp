//
// Created by wilbert on 2026/1/16.
//

#include "LoopThread.h"
#include "Log.hpp"
#include <chrono>
#include <pthread.h>

namespace face
{
    LoopThread::LoopThread(const std::string& name) : mThreadName(name) {}
    
    LoopThread::~LoopThread() {
        stop();
    }
    
    // ============ 监听器设置 ============
    
    void LoopThread::setOnStartListener(OnStart listener) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mOnStartListener = std::move(listener);
    }
    
    void LoopThread::setOnLoopListener(OnLoop listener) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mOnLoopListener = std::move(listener);
    }
    
    void LoopThread::setOnStopListener(OnStop listener) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mOnStopListener = std::move(listener);
    }
    
    // ============ 线程控制 ============
    
    bool LoopThread::start() {
        // 如果已经在运行，返回false
        LOGE("LoopThread::%s %s", __FUNCTION__, mThreadName.c_str());
        if (mRunning.load() || mStartRunning.load()) {
            LOGE("LoopThread::%s %s twice error", __FUNCTION__, mThreadName.c_str());
            return false;
        }
        mStartRunning.store(true);
        // 重置停止标志和请求ID
        mStopRequested.store(false);
        mCurrentRequestId.store(0);
        // 创建并启动线程
        mThread = std::thread(&LoopThread::threadMain, this);
        return true;
    }
    
    void LoopThread::stop() {
        LOGE("LoopThread::%s %s", __FUNCTION__, mThreadName.c_str());
        requestStop();
        join();
    }
    
    void LoopThread::requestStop() {
        LOGE("LoopThread::%s %s, mRunning:%d", __FUNCTION__, mThreadName.c_str(), mRunning.load());
        mStopRequested.store(true);
        // 唤醒可能在等待的线程
        mCondVar.notify_all();
    }
    
    void LoopThread::join() {
        if (mThread.joinable()) {
            mThread.join();
        }
    }
    
    // ============ 状态查询 ============
    
    bool LoopThread::isRunning() const {
        return mRunning.load();
    }

    bool LoopThread::isStartRunning() const {
        return mStartRunning.load();
    }
    
    bool LoopThread::isStopRequested() const {
        return mStopRequested.load();
    }
    
    std::thread::id LoopThread::getThreadId() const {
        return mThread.get_id();
    }
    
    void LoopThread::setLoopMode(LoopMode mode) {
        LoopMode oldMode = mLoopMode.load();
        mLoopMode.store(mode);
        
        // 如果模式改变，唤醒线程以应用新模式
        if (oldMode != mode) {
            mCondVar.notify_one();
        }
    }
    
    LoopMode LoopThread::getLoopMode() const {
        return mLoopMode.load();
    }
    
    void LoopThread::setLoopInterval(uint32_t intervalMs) {
        mLoopIntervalMs.store(intervalMs);
    }
    
    uint32_t LoopThread::getLoopInterval() const {
        return mLoopIntervalMs.load();
    }
    
    void LoopThread::requestLoop(uint64_t requestId) {
        if (mLoopMode.load() == LoopMode::REQUEST) {
            if (!mRunning.load()) {
                LOGE("LoopThread::%s, %s when thread not running, requestId:%d", __FUNCTION__, mThreadName.c_str(), requestId);
            }
            mCurrentRequestId.store(requestId);
            mCondVar.notify_one();
        }
    }
    
    // ============ 线程主函数 ============
    
    void LoopThread::threadMain() {
        if (!mThreadName.empty()) {
#if defined(__APPLE__)
            pthread_setname_np(mThreadName.c_str());
#elif defined(__ANDROID__)
            pthread_setname_np(pthread_self(), mThreadName.c_str());
#endif
        }
        // 标记线程已启动
        mStartRunning.store(false);
        mRunning.store(true);
        
        // 调用启动监听器
        {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            if (mOnStartListener) {
                mOnStartListener();
            }
        }
        
        // 主循环
        while (!mStopRequested.load()) {
            LoopMode currentMode = mLoopMode.load();
            uint64_t requestId = 0;
            
            if (currentMode == LoopMode::REQUEST) {
                // REQUEST模式：等待请求触发
                std::unique_lock<std::mutex> lock(mCondMutex);
                mCondVar.wait(lock, [this]() {
                    return mStopRequested.load() || mCurrentRequestId.load() != 0;
                });
                
                // 如果是因为停止请求而唤醒，退出循环
                if (mStopRequested.load()) {
                    break;
                }
                
                // 获取并重置请求ID
                requestId = mCurrentRequestId.exchange(0);
            }
            
            // 调用循环监听器
            {
                std::lock_guard<std::mutex> lock(mCallbackMutex);
                if (mOnLoopListener) {
                    mOnLoopListener(requestId);
                }
            }
            
            // 如果是INTERVAL模式，根据设置的间隔时间休眠
            if (currentMode == LoopMode::INTERVAL) {
                uint32_t intervalMs = mLoopIntervalMs.load();
                if (intervalMs > 0) {
                    std::unique_lock<std::mutex> lock(mCondMutex);
                    mCondVar.wait_for(lock, std::chrono::milliseconds(intervalMs),
                                      [this]() { return mStopRequested.load() || mLoopMode.load() != LoopMode::INTERVAL; });
                } else {
                    // 不休眠，但检查是否需要让出CPU
                    std::this_thread::yield();
                }
            }
        }
        
        // 调用结束监听器
        {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            if (mOnStopListener) {
                mOnStopListener();
            }
        }
        // 标记线程已停止
        mRunning.store(false);
    }
    
} // face