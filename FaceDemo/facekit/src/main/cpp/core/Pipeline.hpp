//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_PIPELINE_HPP
#define FACEDEMO_PIPELINE_HPP
#include "Processor.hpp"
#include "io/Source.hpp"
#include "io/Destination.hpp"
#include "common/LimitQueue.hpp"
#include "common/LoopThread.h"

#include <vector>

namespace face{

    template<typename I, typename O>
    class Pipeline {
    public:
        virtual ~Pipeline() = default;

        Error start(std::shared_ptr<Source<I>> source, std::shared_ptr<Destination<O>> destination, std::shared_ptr<Processor<I, O>> processor = nullptr) {
            if (!source || !destination || !processor) {
                return Error::Err_InvalidInput;
            }
            mSource = source;
            mDestination = destination;
            mProcessor = processor;
            mThread = std::make_shared<LoopThread>();
            mThread->setLoopMode(LoopMode::REQUEST);

            std::weak_ptr<LimitQueue<I>> weakQueue(mInputQueue);
            std::weak_ptr<Processor<I,O>> weakProcessor(mProcessor);
            std::weak_ptr<Destination<O>> weakDestination(mDestination);
            I inputData;
            mThread->setOnLoopListener([weakQueue,weakProcessor,weakDestination, &inputData] (uint32_t requestCode) -> void {
                auto queue = weakQueue.lock();
                if (queue) {
                    auto res = queue->pop(inputData);
                    auto processor = weakProcessor.lock();
                    if (res == Error::None && processor) {
                        auto outputData = processor(inputData);
                        auto destination = weakDestination.lock();
                        if (destination) {
                            destination->onOutput(outputData);
                        }
                    }
                }
            });

            mInputQueue = std::make_shared<LimitQueue<I>>();
            mInputQueue->setMaxSize(1);
            mInputQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
            mInputQueue->setPushListener([&](const I& input) -> void {
                notifyDataChanged();
            });

            return onStart();
        };

        Error stop() {
            auto res = onStop();
            mSource.reset();
            mDestination.reset();
            mProcessor.reset();
            return res;
        };
    protected:
        virtual Error onStart() {
            if (!mSource || !mDestination || !mInputQueue) {
                return Error::Err_InvalidSource;
            }

            std::weak_ptr<LimitQueue<I>> weakQueue(mInputQueue);
            mSource->setDataAvailableListener([weakQueue] (const I& input) -> void {
                if (auto queue = weakQueue.lock()) {
                    queue->push(input);
                }
            });

            mThread->start();
            return Error::None;
        };

        virtual void notifyDataChanged() {
            if (mThread) {
                mThread->requestLoop(++requestId);
            }
        }
        virtual Error onStop() {
            mSource->setDataAvailableListener(nullptr);
            mThread->requestStop();
        };

        std::shared_ptr<Source<I>> mSource;
        std::shared_ptr<Destination<O>> mDestination;
        std::shared_ptr<Processor<I, O>> mProcessor;
        std::shared_ptr<LimitQueue<I>> mInputQueue;
        std::shared_ptr<LoopThread> mThread;
        uint32_t requestId = 0;
    };
}
#endif //FACEDEMO_PIPELINE_HPP
