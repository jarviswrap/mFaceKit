//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_CONSUMER_HPP
#define FACEDEMO_CONSUMER_HPP
#include "common/Common.hpp"
#include <atomic>

namespace face{
    template<typename T>
    class Consumer {
    public:
        virtual ~Consumer() = default;
        virtual T getNextData() = 0;

        Error requestConsume(uint32_t requestId = 1) {
            mRequestId.store(requestId);
            return onRequestConsume(requestId);
        };

        virtual void setConsumeListener(DataObtainer<T> listener) {
            mDataObtainer = std::move(listener);
        };

        virtual Error consumeData() {
            if (mDataObtainer == nullptr) {
                return Error::Err_InvalidInput;
            }
            return onConsumeData(mDataObtainer(mRequestId.load()));
        }
    protected:
        DataObtainer<T> mDataObtainer;
        std::atomic<uint32_t> mRequestId{0};

        virtual Error onRequestConsume(uint32_t requestId) = 0;
        virtual Error onConsumeData(T data) = 0;
    };
} // face
#endif //FACEDEMO_CONSUMER_HPP
