//
// Created by wilbert on 2026/1/15.
//

#include "ImageProcessor.hpp"
#include "common/Log.hpp"

#include <memory>

namespace face
{
    ImageProcessor::ImageProcessor() {
        mDataQueue = std::make_shared<LimitQueue<std::shared_ptr<PixelData>>>();
        mDataQueue->setMaxSize(2);
        mThread = std::make_shared<LoopThread>();
        mThread->setLoopMode(LoopMode::REQUEST);
        std::weak_ptr<LimitQueue<std::shared_ptr<PixelData>>> weakPtr(mDataQueue);

        std::shared_ptr<PixelData> cacheData;
        mThread->setOnLoopListener([weakPtr, &cacheData](uint32_t requestId) -> void {
            LOGE("onLoop requestId:%u", requestId);
            if (auto dataQueue = weakPtr.lock()) {
                if (dataQueue->pop(cacheData) == Error::None) {
                    LOGE("onDataAvailable:%zu", cacheData->getPixelSize());
                }
            }
        });
    }

    ImageProcessor::~ImageProcessor() {
        release();
    }

    Error ImageProcessor::init(const std::string &model, int forward, int numThread) {
        auto interpreter = MNN::Interpreter::createFromFile(model.c_str());
        if (interpreter) {
            mInterpreter.reset(interpreter);
            MNN::ScheduleConfig config;
            config.type = (MNNForwardType) forward;
            if (numThread > 0) {
                config.numThread = numThread;
            }
            mSession = mInterpreter->createSession(config);
            auto tensor = mInterpreter->getSessionInput(mSession, nullptr);
            if (tensor) {
                mInputTensor.reset(tensor);
            }
            return Error::None;
        }
        return Error::Err_ModelInvalid;
    }

    Error ImageProcessor::release() {
        if (mSession) {
            mInterpreter->releaseSession(mSession);
            mSession = nullptr;
        }
        if (mInterpreter) {
            MNN::Interpreter::destroy(mInterpreter.get());
            mInterpreter.reset();
        }
        if (mSource) {
            mSource->setDataAvailableListener(nullptr);
        }
        return Error::None;
    }

    Error ImageProcessor::start(const std::shared_ptr<Source<PixelData>>& source) {
        if (!mInterpreter) {
            return Error::Err_InvalidInterpreter;
        }
        if (source) {
            mSource = source;
            mThread->start();
            std::weak_ptr<LimitQueue<std::shared_ptr<PixelData>>> weakPtr(mDataQueue);
            std::weak_ptr<LoopThread> weakThread(mThread);
            auto requestId = ++mRequestId;
            source->setDataAvailableListener([weakPtr, weakThread, requestId] (const std::shared_ptr<PixelData>& pixelData) -> void {
                if (auto dataQueue = weakPtr.lock()) {
                    dataQueue->push(pixelData);
                }
                if (auto thread = weakThread.lock()) {
                    thread->requestLoop(requestId);
                }
            });
            return Error::None;
        }
        return Error::Err_InvalidSource;
    }

    Error ImageProcessor::stop() {
        if (mSource) {
            mSource->setDataAvailableListener(nullptr);
        }
        mThread->stop();
        return Error::None;
    }
} // engine