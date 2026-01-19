//
// Created by wilbert on 2026/1/17.
//

#ifndef FACEDEMO_DESTINATION_HPP
#define FACEDEMO_DESTINATION_HPP
#include "common/LimitQueue.hpp"


namespace face {
    template<typename T>
    class Destination {
    public:
        Destination() {
            mDataQueue.setMaxSize(2);
            mDataQueue.setLimitPolicy(LimitPolicy::WaitWhenBusy);
        }
        virtual ~Destination() = default;
        virtual Error onOutput(const T& outputData) {
            mDataQueue.push(outputData);
        };

    protected:
        LimitQueue<T> mDataQueue;
    };
}
#endif //FACEDEMO_DESTINATION_HPP
