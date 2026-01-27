//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_SOURCE_HPP
#define FACEDEMO_SOURCE_HPP

#include "common/Common.hpp"
#include <memory>

namespace face {
    template<typename T>
    class Source {
    public:
        virtual ~Source() = default;
        virtual std::shared_ptr<T> getNextData() = 0;

        virtual bool isDataAvailable() { return false; };
        virtual void setDataAvailableListener(DataListener<std::shared_ptr<T>> listener) {
            mDataListener = std::move(listener);
        };
    protected:
        DataListener<std::shared_ptr<T>> mDataListener{nullptr};
    };
}
#endif //FACEDEMO_SOURCE_HPP
