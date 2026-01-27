//
// Created by wilbert on 2026/1/21.
//

#ifndef FACEDEMO_RENDER_HPP
#define FACEDEMO_RENDER_HPP

#include "common/Common.hpp"
#include "common/RenderData.hpp"

namespace face {
    template<typename T>
    class Render { // Render只在当前环境上直接绘制，不会包含FBO

    public:
        Render() = default;
        virtual ~Render() = default;

        virtual void resize(int width, int height) = 0;
        virtual Error render(const std::shared_ptr<RenderData<T>> &data) = 0;
        virtual void destroy() = 0;
    };
} // face
#endif //FACEDEMO_RENDER_HPP
