//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_POSITION_HPP
#define FACEDEMO_POSITION_HPP

namespace face {

    template<typename T>
    class Position {
    public:
        Position(T _x, T _y): x(_x), y(_y) {}
        Position() = default;
        ~Position() = default;
        T                      x{0};
        T                      y{0};

        void get(T& _x, T& _y) const {
            _x = x;
            _y = y;
        }
    };

} // face

#endif //FACEDEMO_POSITION_HPP
