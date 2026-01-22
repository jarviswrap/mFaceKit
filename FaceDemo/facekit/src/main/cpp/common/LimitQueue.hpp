#pragma once

#include "common/Common.hpp"
#include <deque>
#include <mutex>
#include <condition_variable>

namespace face {
    enum class LimitPolicy {
        DropWhenBusy, WaitWhenBusy
    };

    template<typename T>
    class LimitQueue {
    public:
        LimitQueue() = default;

        ~LimitQueue() = default;

        // 禁用拷贝构造和赋值操作
        LimitQueue(const LimitQueue &) = delete;

        LimitQueue &operator=(const LimitQueue &) = delete;

        void setLimitPolicy(LimitPolicy policy) { mPolicy = policy; }
        
        bool setMaxSize(size_t maxSize) {
            std::lock_guard<std::mutex> lock(mMutex);
            if (maxSize == 0) {
                maxSize = 1;
            }
            if (mMaxSize != maxSize) {
                mMaxSize = maxSize;
                // 立即检查并移除多余元素
                while (mQueue.size() > mMaxSize) {
                    if (mPopListener) {
                        mPopListener(mQueue.front());
                    }
                    mQueue.pop_front();
                }
                mCond.notify_all(); // 唤醒等待的线程
                return true;
            }
            return false;
        }

        /**
         * @param popListener(bool: 是否自动移除数据，T: 被移除的数据)
         */
        void setPopListener(DataListener<std::shared_ptr<T>> &&popListener = nullptr) {
            mPopListener = popListener;
        }

        // 使用函数模板实现通用引用
        template<typename U,
                 typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
        size_t push(std::shared_ptr<U> &&item) {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mPolicy == LimitPolicy::WaitWhenBusy) {
                mCond.wait(lock, [this] { return mQueue.size() < mMaxSize; });
            } else { // DropWhenBusy
                while (mQueue.size() >= mMaxSize) {
                    if (mPopListener) {
                        mPopListener(mQueue.front());
                    }
                    mQueue.pop_front();
                }
            }
            mQueue.push_back(std::move(item));
            return mQueue.size();
        }

        template<typename U,
                 typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
        size_t push(const std::shared_ptr<U> &item) {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mPolicy == LimitPolicy::WaitWhenBusy) {
                mCond.wait(lock, [this] { return mQueue.size() < mMaxSize; });
            } else { // DropWhenBusy
                while (mQueue.size() >= mMaxSize) {
                    if (mPopListener) {
                        mPopListener(mQueue.front());
                    }
                    mQueue.pop_front();
                }
            }
            mQueue.push_back(item);
            return mQueue.size();
        }

        std::shared_ptr<T> pop() {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mQueue.empty()) {
                return nullptr;
            }
            auto item = mQueue.front();
            mQueue.pop_front();
            lock.unlock(); /**如果不手动解锁， mCond.notify_one() 会唤醒等待的线程。被唤醒的线程会尝试重新获取 mMutex 锁，但此时锁仍然被当前线程持有（因为还没退出作用域），导致被唤醒的线程立即又阻塞了（Hurry up and wait）。 手动解锁 后，再发出通知，等待的线程醒来时可以直接拿到锁，从而提高并发效率。*/
            mCond.notify_one();
            return item;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mMutex);
            return mQueue.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(mMutex);
            return mQueue.size();
        }

        size_t capacity() const {
            return mMaxSize;
        }

        void clear() {
            std::lock_guard<std::mutex> lock(mMutex);
            while (!mQueue.empty()) {
                if (mPopListener) {
                    mPopListener(mQueue.front());
                }
                mQueue.pop_front();
            }
            mCond.notify_all();
        }

        const std::deque<std::shared_ptr<T>> &data() const { return mQueue; };

    private:
        LimitPolicy mPolicy{LimitPolicy::DropWhenBusy};
        std::deque<std::shared_ptr<T>> mQueue;
        mutable std::mutex mMutex;
        std::condition_variable mCond;
        size_t mMaxSize{1};
        DataListener<std::shared_ptr<T>> mPopListener{nullptr};
    };

}