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

        void get(T& x, T& y, T& w, T& h) {
            x = startX;
            y = startY;
            w = width;
            h = height;
        }

        bool set(T x, T y, T w, T h) {
            if (x != startX || y != startY || w != width || h != height) {
                startX = x;
                startY  = y;
                width   = w;
                height = h;
                return true;
            }
            return false;
        }

        bool set(T w, T h) {
            if (w != width || h != height) {
                width = w;
                height = h;
                return true;
            }
            return false;
        }

        bool setPosition(T x, T y) {
            if (x != startX || y != startY) {
                startX = x;
                startY = y;
                return true;
            }
            return false;
        }

        bool isValid() { return width > 0 && height > 0; }

        T startX{0};
        T startY{0};
        T width{0};
        T height{0};
    };

} // face

#endif //FACEDEMO_RECT_HPP
