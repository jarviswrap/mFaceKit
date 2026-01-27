//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_LOOPTHREAD_H
#define FACEDEMO_LOOPTHREAD_H

#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>
#include "common/Common.hpp"

namespace face
{
    /**
     * 循环模式
     */
    enum class LoopMode {
        REQUEST,    // 请求触发模式：需要主动调用requestLoop()触发循环
        INTERVAL,   // 间隔触发模式：按固定间隔自动触发循环

    };
    
    /**
     * 消息循环线程类
     * 提供启动、循环和结束三个事件的回调机制
     * 支持两种循环模式：请求触发和固定间隔触发
     */
    class LoopThread
    {
    public:
        // 回调函数类型定义 - 使用DataListener模板
        // OnStart: 线程启动事件，无参数
        using OnStart = DataListener<>;
        // OnLoop: 循环事件，参数为requestId
        using OnLoop = DataListener<uint64_t>;
        // OnStop: 线程结束事件，无参数
        using OnStop = DataListener<>;
        
        LoopThread(const std::string& name = "LoopThread");
        virtual ~LoopThread();
        
        // 禁止拷贝
        LoopThread(const LoopThread&) = delete;
        LoopThread& operator=(const LoopThread&) = delete;
        
        // ============ 回调设置 ============
        
        /**
         * 设置线程启动监听器
         * 在线程启动后、进入循环前调用
         */
        void setOnStartListener(OnStart listener);
        
        /**
         * 设置循环监听器
         * 在每次循环迭代中调用
         * @param listener 接收requestId参数的监听器
         */
        void setOnLoopListener(OnLoop listener);
        
        /**
         * 设置线程结束监听器
         * 在退出循环后、线程结束前调用
         */
        void setOnStopListener(OnStop listener);
        
        // ============ 线程控制 ============
        
        /**
         * 启动线程
         * @return true 成功启动，false 线程已在运行
         */
        bool start();
        
        /**
         * 停止线程
         * 会等待线程完全结束
         */
        void stop();
        
        /**
         * 请求停止线程（异步）
         * 不等待线程结束
         */
        void requestStop();
        
        /**
         * 等待线程结束
         */
        void join();
        
        // ============ 状态查询 ============
        
        /**
         * 检查线程是否正在运行
         */
        bool isRunning() const;

        /**
         * 检查线程是否正在准备运行
         */
        bool isStartRunning() const;

        /**
         * 检查是否已请求停止
         */
        bool isStopRequested() const;
        
        /**
         * 获取线程ID
         */
        std::thread::id getThreadId() const;
        
        /**
         * 设置循环模式
         * @param mode 循环模式（REQUEST或INTERVAL）
         */
        void setLoopMode(LoopMode mode);
        
        /**
         * 获取当前循环模式
         */
        LoopMode getLoopMode() const;
        
        /**
         * 设置循环间隔时间（毫秒）
         * 仅在INTERVAL模式下有效
         * @param intervalMs 循环间隔，0表示不休眠
         */
        void setLoopInterval(uint32_t intervalMs);
        
        /**
         * 获取循环间隔时间
         */
        uint32_t getLoopInterval() const;
        
        /**
         * 请求执行一次循环
         * 仅在REQUEST模式下有效
         * 唤醒线程执行一次循环回调
         * @param requestId 请求ID，会在循环回调中传递
         */
        void requestLoop(uint64_t requestId = 1);
        
    protected:
        /**
         * 线程主函数
         * 可由子类重写以实现自定义行为
         */
        virtual void threadMain();
        
    private:
        // 线程对象
        std::thread mThread;
        std::string mThreadName;
        
        // 线程状态
        std::atomic<bool> mRunning{false};
        std::atomic<bool> mStartRunning{false};
        std::atomic<bool> mStopRequested{false};
        
        // 监听器
        OnStart mOnStartListener;
        OnLoop mOnLoopListener;
        OnStop mOnStopListener;
        
        // 循环模式
        std::atomic<LoopMode> mLoopMode{LoopMode::INTERVAL};
        
        // 循环间隔（毫秒）
        std::atomic<uint32_t> mLoopIntervalMs{50};
        
        // 请求ID（用于REQUEST模式）
        // -1表示无请求，>=0表示有效请求
        std::atomic<uint64_t> mCurrentRequestId{0};
        
        // 用于保护回调函数的互斥锁
        mutable std::mutex mCallbackMutex;
        
        // 用于线程同步的条件变量
        std::condition_variable mCondVar;
        std::mutex mCondMutex;
    };

} // face

#endif //FACEDEMO_LOOPTHREAD_H
