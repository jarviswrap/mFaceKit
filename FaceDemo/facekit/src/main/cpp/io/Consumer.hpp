//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_CONSUMER_HPP
#define FACEDEMO_CONSUMER_HPP
#include "common/Common.hpp"
#include <atomic>
#include <memory>

namespace face{
    template<typename T>
    class Consumer {
    public:
        virtual ~Consumer() = default;

        // requestConsume(uint32_t requestId = 1)应该和destroy()相同线程环境（生产者线程）
        Error requestConsume(uint32_t requestId = UINT32_MAX) {
            mRequestId.store(requestId);
            return onRequestConsume(requestId);
        };

        virtual void setConsumeListener(DataObtainer<std::shared_ptr<T>> listener) {
            mDataObtainer = std::move(listener);
        };

        virtual Error consumeData() {
            if (mDataObtainer == nullptr) {
                return Error::Err_InvalidData;
            }
            return onConsumeData(mDataObtainer(mRequestId.load()));
        }

        // destroy()应该和requestConsume(uint32_t requestId = 1)相同线程环境（生产者线程）
        void destroy() {
            onDestroy();
        };
    protected:
        DataObtainer<std::shared_ptr<T>> mDataObtainer{nullptr};
        std::atomic<uint32_t> mRequestId{0};

        virtual Error onRequestConsume(uint32_t requestId) = 0; // onRequestConsume和onDestroy都来自生产者线程
        virtual void onDestroy() {};

        virtual Error onConsumeData(const std::shared_ptr<T>& data) = 0; //onConsumeData来自消费者线程
    };
} // face
#endif //FACEDEMO_CONSUMER_HPP
