//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_SOURCE_HPP
#define FACEDEMO_SOURCE_HPP

#include <utility>

#include "../common/Common.hpp"

namespace face {
    template<typename T>
    class Source {
    public:
        virtual ~Source() = default;
        virtual T getNextData() = 0;

        virtual bool isDataAvailable() { return false; };
        virtual void setDataAvailableListener(DataListener<T> listener) {
            mDataListener = std::move(listener);
        };
    protected:
        DataListener<T> mDataListener;
    };
}
#endif //FACEDEMO_SOURCE_HPP
