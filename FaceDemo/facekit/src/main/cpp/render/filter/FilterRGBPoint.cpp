#include "FilterRGBPoint.hpp"
#include "render/utils/OpenGLUtils.hpp"
#include <string>
#include <algorithm>
#include "common/Log.hpp"

namespace face {
    const static std::string sRgbaPointVertexShaderCode = R"(
    attribute vec2 aPosition;
    void main() {
      gl_PointSize = 2.0;
      gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
    }
    )";

    const static std::string sRgbaPointFragmentShaderCode = R"(
    precision highp float;
    uniform vec4 color;
    void main() {
        gl_FragColor = color;
    }
    )";

    FilterRGBPoint::~FilterRGBPoint() {
        if (mProgID > 0) {
            glDeleteProgram(mProgID);
            mProgID = 0;
        }
    }

    bool FilterRGBPoint::init() {
        mProgID = OpenGLUtils::loadProgram(sRgbaPointVertexShaderCode.c_str(),
                                           sRgbaPointFragmentShaderCode.c_str());
        if (mProgID == 0) {
            LOGE("FilterRGBPoint loadProgram Fail");
            return false;
        }
        maPosition = glGetAttribLocation(mProgID, "aPosition");
        if (maPosition < 0) {
            LOGE("FilterRGBPoint get aPosition Fail");
            return false;
        }
        mColorUniform = glGetUniformLocation(mProgID, "color");
        if (mColorUniform < 0) {
            LOGE("FilterRGBPoint get colore Fail");
            return false;
        }
        return true;
    }

    void FilterRGBPoint::draw(const Vec3 &color) {
        const Vec2 pos[4] = {
                {-1.0f, -1.0f},
                {-1.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, -1.0f},
        };

        glUseProgram(mProgID);
        glVertexAttribPointer(maPosition, 2, GL_FLOAT, false, 0, (const void *) &pos[0]);
        glEnableVertexAttribArray(maPosition);
        glUniform4f(mColorUniform, color.x, color.y, color.z, 1.0f);
        glDrawArrays(GL_POINTS, 0, 4);
        glDisableVertexAttribArray(maPosition);
    }

    bool FilterRGBPoint::isInitialized() {
        return mProgID > 0;
    }

}  //namespace pipeline
