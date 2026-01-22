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
            setConsumer(consumer);
        }

        virtual ~Destination() = default;

        void setConsumer(std::shared_ptr<Consumer<T>> consumer) {
            if (!consumer) {
                return;
            }
            if (mConsumer) {
                mConsumer->setConsumeListener(nullptr);
                mConsumer.reset();
            }
            mConsumer = consumer;
            std::weak_ptr<LimitQueue<T>> weakQueue = mDataQueue;
            mConsumer->setConsumeListener([weakQueue] (uint32_t requestId) -> std::shared_ptr<T> {
                if (auto queue = weakQueue.lock()) {
                    return queue->pop();
                }
                return nullptr;
            });
        }

        virtual Error output(const std::shared_ptr<T>& outputData) {
            mDataQueue->push(outputData);
            auto consumer = mConsumer;
            if (consumer) {
                return consumer->requestConsume(++mRequestId);
            }
            return Error::Err_InvalidConsumer;
        };

        virtual void destroy() {
            auto consumer = mConsumer;
            if (consumer) {
                consumer->destroy();
            }
        };

    protected:
        uint32_t mRequestId{0};
        std::shared_ptr<LimitQueue<T>> mDataQueue{nullptr};
        std::shared_ptr<Consumer<T>> mConsumer{nullptr};
    };
}
#endif //FACEDEMO_DESTINATION_HPP
