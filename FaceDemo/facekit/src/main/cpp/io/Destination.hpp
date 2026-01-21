//
// Created by wilbert on 2026/1/17.
//

#ifndef FACEDEMO_DESTINATION_HPP
#define FACEDEMO_DESTINATION_HPP
#include "common/LimitQueue.hpp"
#include "Consumer.hpp"


namespace face {
    template<typename T>
    class Destination {
    public:
        explicit Destination(std::shared_ptr<Consumer<T>> consumer) {
            mDataQueue = std::make_shared<LimitQueue<T>>();
            mDataQueue->setMaxSize(2);
            mDataQueue->setLimitPolicy(LimitPolicy::WaitWhenBusy);
            mConsumer = consumer;
            std::weak_ptr<LimitQueue<T>> weakQueue = mDataQueue;
            mConsumer->setConsumeListener([weakQueue] (uint32_t requestId) -> std::shared_ptr<T> {
                if (auto queue = weakQueue.lock()) {
                    return queue->pop();
                }
                return nullptr;
            });
        }

        virtual ~Destination() = default;
        virtual Error output(const std::shared_ptr<T>& outputData) {
            mDataQueue->push(outputData);
            if (mConsumer) {
                return mConsumer->requestConsume();
            }
            return Error::Err_InvalidConsumer;
        };

        virtual void destroy() {
            if (mConsumer) {
                mConsumer->destroy();
            }
        };

    protected:
        std::shared_ptr<LimitQueue<T>> mDataQueue{nullptr};
        std::shared_ptr<Consumer<T>> mConsumer{nullptr};
    };
}
#endif //FACEDEMO_DESTINATION_HPP
