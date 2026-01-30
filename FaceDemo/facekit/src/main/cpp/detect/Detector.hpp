//
// Created by wilbert on 2026/1/30.
//

#ifndef FACEDEMO_DETECTOR_HPP
#define FACEDEMO_DETECTOR_HPP
#include <memory>

namespace face {

    template <typename I, typename O>
    class Detector {

    public:
        virtual void detect(const std::shared_ptr<I>& data) = 0;
        virtual std::shared_ptr<O> getDetectResult(const std::shared_ptr<I>& data, uint32_t timeout) = 0;
    };

} // face

#endif //FACEDEMO_DETECTOR_HPP
