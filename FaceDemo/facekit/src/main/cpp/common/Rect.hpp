//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_RECT_HPP
#define FACEDEMO_RECT_HPP

namespace face {

    template<typename T>
    class Rect {
    public:
        Rect(T x, T y, T w, T h): startX(x), startY(y), width(w), height(h) {}
        Rect() = default;
        ~Rect() = default;

        void set(T x, T y, T w, T h) {
            startX = x;
            startY  = y;
            width   = w;
            height = h;
        }

        bool isValid() { return width > 0 && height > 0; }

        T startX{0};
        T startY{0};
        T width{0};
        T height{0};
    };

} // face

#endif //FACEDEMO_RECT_HPP
