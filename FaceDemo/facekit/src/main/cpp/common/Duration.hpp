//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_DURATION_HPP
#define FACEDEMO_DURATION_HPP

#include <stdint.h>

namespace face {

    class Duration {
    public:
        Duration(uint64_t s, uint64_t e): start(s), end(e) {}
        Duration() = default;
        ~Duration() = default;

        uint64_t start{0};
        uint64_t end{0};

        void set(uint64_t s, uint64_t e) {
            start = s;
            end = e;
        }

        uint64_t duration() const { return end - start; }

        bool isActive(uint64_t timeStamp) const { return timeStamp >= start && timeStamp < end; }
    };

} // face

#endif //FACEDEMO_DURATION_HPP
