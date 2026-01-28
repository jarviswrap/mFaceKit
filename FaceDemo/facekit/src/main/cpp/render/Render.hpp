//
// Created by wilbert on 2026/1/21.
//

#ifndef FACEDEMO_RENDER_HPP
#define FACEDEMO_RENDER_HPP

#include "common/Common.hpp"
#include <memory>
namespace face {
    template<typename T>
    class Render {

    public:
        Render() = default;
        virtual ~Render() = default;

        virtual void onSurfaceChanged(int width, int height) = 0;
        virtual Error onDrawFrame(const std::shared_ptr<T> &data) = 0;
        virtual void onDestroy() = 0;
    };
} // face
#endif //FACEDEMO_RENDER_HPP
