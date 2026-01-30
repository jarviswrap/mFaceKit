//
// Created by wilbert on 2026/1/27.
//

#ifndef FACEDEMO_RENDERDATA_HPP
#define FACEDEMO_RENDERDATA_HPP

#include <stdint.h>
#include "Rect.hpp"
#include <memory>
namespace face {



    template <typename T>
    class RenderData {
    public:
        Rect<uint32_t> rect;
        float rotation{0};

        std::shared_ptr<T> data{nullptr};
    };

} // face

#endif //FACEDEMO_RENDERDATA_HPP
