//
// Created by wilbert on 2026/1/26.
//

#include "Timeline.hpp"
#include "common/Log.hpp"

namespace face {
    Timeline::Timeline(bool autoTick): mAutoTick(autoTick) {}

    Timeline::~Timeline() {
        stop();
    }

    void Timeline::start() {
        if (!mThread) {
            mThread = std::make_shared<LoopThread>("Timeline");
        }
        LOGE("Timeline::%s, mAutoTick:%d, this:%p", __FUNCTION__, mAutoTick, this);
        mThread->setLoopMode(mAutoTick?LoopMode::INTERVAL: LoopMode::REQUEST);
        if (mAutoTick) {
            mThread->setLoopInterval(TICK_INTERVAL);
        }
        std::weak_ptr<Timeline> weakPtr(shared_from_this());
        mThread->setOnLoopListener([weakPtr] (uint64_t requestId) {
            if (auto ptr = weakPtr.lock()) {
                ptr->onTick();
            }
        });
        mCurrentTimeStamp = 0;
        mThread->start();
    }

    void Timeline::stop() {
        if (mThread) {
            mThread->stop();
        }
    }

    int32_t Timeline::addTickListener(DataListener<uint64_t> tickListener) {
        std::lock_guard<std::mutex> lk(mMutex);
        mTickListenerList.push_back(tickListener);
        return mTickListenerList.size();
    }

    void Timeline::removeTickListener(int32_t index) {
        std::lock_guard<std::mutex> lk(mMutex);
        int size = mTickListenerList.size();
        if (index >= size) {
            return;
        }
        mTickListenerList.erase(mTickListenerList.begin() + index);
    }

    void Timeline::onTick() {
        std::lock_guard<std::mutex> lk(mMutex);
        auto currentTime = mCurrentTimeStamp.load();
        for (auto& ticker: mTickListenerList) {
            ticker(currentTime);
        }
        mCurrentTimeStamp += TICK_INTERVAL;
    }

    void Timeline::tick() {
        if (mThread) {
            mThread->requestLoop();
        }
    }
} // face