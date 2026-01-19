//
// Created by wilbert on 2026/1/17.
//

#ifndef FACEDEMO_PROCESSOR_HPP
#define FACEDEMO_PROCESSOR_HPP
#include "io/Source.hpp"
#include "io/Destination.hpp"

namespace face {
    template<typename I, typename O>
    class Processor {
    public:
        virtual O&& onProcess(I&& input) = 0;
    };

    template<typename T>
    class PassProcessor: Processor<T, T> {
        T&& onProcess(T&& input) override { return std::move(input); };
    };
} // face

#endif //FACEDEMO_PROCESSOR_HPP
