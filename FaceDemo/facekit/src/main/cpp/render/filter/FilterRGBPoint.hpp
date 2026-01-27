#pragma once

#include "render/utils/GLType.hpp"
#include <vector>

namespace face {
/**
 * 绘制点
 */
    class FilterRGBPoint {
    public:
        typedef struct vec2 {
            vec2() = default;

            vec2(float xValue, float yValue) : x(xValue), y(yValue) {}

            float x = {0};
            float y = {0};
        } Vec2;

        typedef struct vec3 {
            vec3() = default;

            vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}

            float x = {0};
            float y = {0};
            float z = {0};
        } Vec3;

        FilterRGBPoint() = default;
        ~FilterRGBPoint();

        bool init();
        bool isInitialized();
        void draw(const Vec3 &color);

    private:
        GLint  maPosition    = -1;
        GLint  mColorUniform = -1;
        GLuint mProgID       = GL_NONE;
    };

}  //namespace pipeline
