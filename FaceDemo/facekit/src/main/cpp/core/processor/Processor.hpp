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
        std::shared_ptr<O> process(const std::shared_ptr<I>& input) {
            return onProcess(input);
        };

    protected:
        virtual std::shared_ptr<O> onProcess(const std::shared_ptr<I>& input) = 0;

    };

    template<typename T>
    class PassProcessor: public Processor<T, T> {
    public:
        std::shared_ptr<T> onProcess(const std::shared_ptr<T>& input) override {
            return input;
        };
    };

    
} // face

#endif //FACEDEMO_PROCESSOR_HPP
