//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_TIMELINE_HPP
#define FACEDEMO_TIMELINE_HPP
#include "common/Common.hpp"
#include "common/LoopThread.h"
#include <atomic>
#include <vector>
namespace face {

    class Timeline : public std::enable_shared_from_this<Timeline>{

    public:
        Timeline(bool autoTick = true);
        ~Timeline();

        void start();
        void stop();

        void tick();

        uint64_t getCurrentTime() { return mCurrentTimeStamp.load(); };
        int32_t addTickListener(DataListener<uint64_t> tickListener);
        void removeTickListener(int32_t index);
        bool isAutoTick() const { return mAutoTick; }
    private:
        bool mAutoTick{true};
        static constexpr uint64_t TICK_INTERVAL = 40;
        std::atomic<uint64_t> mCurrentTimeStamp{0};
        std::shared_ptr<LoopThread> mThread{nullptr};
        std::vector<DataListener<uint64_t>> mTickListenerList;
        std::mutex mMutex;
        void onTick();
    };

} // face

#endif //FACEDEMO_TIMELINE_HPP
