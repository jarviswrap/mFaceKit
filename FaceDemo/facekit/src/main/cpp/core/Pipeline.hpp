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
        Pipeline() = default;
        virtual ~Pipeline() { stop(); };

        Error start(std::shared_ptr<Source<I>> source, std::shared_ptr<Destination<O>> destination, std::shared_ptr<Processor<I, O>> processor) {
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

            mThread->setOnLoopListener([weakQueue,weakProcessor,weakDestination] (uint32_t requestCode) -> void {
                auto queue = weakQueue.lock();
                if (queue) {
                    auto inputData = queue->pop();
                    auto processor = weakProcessor.lock();
                    if (inputData && processor) {
                        auto destination = weakDestination.lock();
                        if (destination) {
                            destination->output(processor->onProcess(std::move(inputData)));
                        }
                    }
                }
            });

            auto dest = mDestination;
            mThread->setOnStopListener([dest] () -> void {
                dest->destroy();
            });

            mInputQueue = std::make_shared<LimitQueue<I>>();
            mInputQueue->setMaxSize(1);
            mInputQueue->setLimitPolicy(LimitPolicy::DropWhenBusy);
            return onStart();
        };

        Error stop() {
            mSource->setDataAvailableListener(nullptr);
            mThread->requestStop();
            mSource.reset();
            mDestination.reset();
            mProcessor.reset();
            return Error::None;
        };
    protected:
        virtual Error onStart() {
            if (!mSource || !mDestination || !mInputQueue) {
                return Error::Err_InvalidSource;
            }

            std::weak_ptr<LimitQueue<I>> weakQueue(mInputQueue);
            mSource->setDataAvailableListener([this, weakQueue] (const std::shared_ptr<I>& inputData) -> void {
                if (auto queue = weakQueue.lock()) {
                    queue->push(std::move(inputData));
                    notifyDataChanged();
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

        std::shared_ptr<Source<I>> mSource;
        std::shared_ptr<Destination<O>> mDestination;
        std::shared_ptr<Processor<I, O>> mProcessor;
        std::shared_ptr<LimitQueue<I>> mInputQueue;
        std::shared_ptr<LoopThread> mThread;
        uint32_t requestId = 0;
    };
}
#endif //FACEDEMO_PIPELINE_HPP
