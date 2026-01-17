#pragma once
#include "common/Common.hpp"
#include <deque>
#include <mutex>

namespace face
{
    template<typename T>
    class LimitQueue
    {
    public:
        LimitQueue() = default;

        ~LimitQueue() = default;

        // 禁用拷贝构造和赋值操作
        LimitQueue(const LimitQueue &) = delete;

        LimitQueue &operator=(const LimitQueue &) = delete;

        bool setMaxSize(size_t maxSize)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (maxSize == 0)
            {
                maxSize = 1;
            }
            if (mMaxSize != maxSize)
            {
                mMaxSize = maxSize;
                // 立即检查并移除多余元素
                while (mQueue.size() > mMaxSize)
                {
                    if (mPopListener)
                    {
                        mPopListener(true, mQueue.front());
                    }
                    mQueue.pop_front();
                }
                return true;
            }
            return false;
        }

        void setPushListener(DataListener<T> &&pushListener = nullptr)
        {
            mPushListener = pushListener;
        }

        /**
         * @param popListener(bool: 是否自动移除数据，T: 被移除的数据)
         */
        void setPopListener(DataListener<bool, T> &&popListener = nullptr)
        {
            mPopListener = popListener;
        }

        // 使用函数模板实现通用引用
        template<typename U,
                typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
        size_t push(U &&item)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mQueue.size() >= mMaxSize)
            {
                if (mPopListener)
                {
                    mPopListener(true, mQueue.front());
                }
                mQueue.pop_front();
            }
            if (mPushListener)
            {
                mPushListener(item);
            }
            mQueue.push_back(std::forward<U>(item));
            return mQueue.size();
        }

        Error pop(T &item)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mQueue.empty())
            {
                return Error::Err_EmptyQueue;
            }
            item = std::move(mQueue.front());
            if (mPopListener)
            {
                mPopListener(false, item);
            }
            mQueue.pop_front();
            return Error::None;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mQueue.empty();
        }

        size_t size() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mQueue.size();
        }

        size_t capacity() const
        {
            return mMaxSize;
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            while (!mQueue.empty())
            {
                if (mPopListener)
                {
                    mPopListener(true, mQueue.front());
                }
                mQueue.pop_front();
            }
        }

        const std::deque<T>& data() const { return mQueue; };

    private:
        std::deque<T>      mQueue;
        mutable std::mutex mMutex;
        size_t             mMaxSize{1};
        DataListener<T>    mPushListener{nullptr};
        DataListener<bool, T>     mPopListener{nullptr};
    };

}