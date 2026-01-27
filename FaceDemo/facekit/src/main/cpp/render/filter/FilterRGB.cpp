#include "FilterRGB.hpp"
#include "common/Log.hpp"
#include "FilterRGBPoint.hpp"

namespace face {
    const static std::string RgbaVertexShaderCode = R"(
    attribute vec2 attPosition;
    attribute vec2 attTexCoord;
    varying vec2 texCoord;
    void main() {
    gl_Position = vec4(attPosition, 0.0, 1.0);
    texCoord =  attTexCoord;
    }
    )";

    const static std::string RgbaFragmentShaderCode = R"(
    precision highp float;
    varying vec2 texCoord;
    uniform sampler2D SamplerRGBA;
    void main() {
            gl_FragColor = texture2D(SamplerRGBA, texCoord);
    }
    )";

    const static std::string hackFragmentShaderCode = R"(
            precision highp float;
            varying vec2 texCoord;
            uniform sampler2D SamplerRGBA;
            uniform vec3 color;
            uniform float width;
            uniform float height;
            void main() {
            float coefficient = step(gl_FragCoord.x, 1.0) * step(gl_FragCoord.y, 1.0) + step(width - 1.0 ,gl_FragCoord.x) * step(height - 1.0,gl_FragCoord.y);
            vec4 texColor = vec4(texture2D(SamplerRGBA, texCoord).rgb, 1.0);
            vec4 hackColor = vec4(color, 1.0);
            gl_FragColor = mix(texColor, hackColor, coefficient) ;
            }
    )";

    FilterRGB::FilterRGB(bool enableColorTest, bool enableColorPoint)
            : mEnableColorTest(enableColorTest), mEnableColorPoint(enableColorPoint) {
    }

    FilterRGB::~FilterRGB() {
        if (mEnableColorTest && mFilterPoint) {
            delete mFilterPoint;
            mFilterPoint = nullptr;
        }
    }

    void FilterRGB::onInit() {
        FilterBase::onInit();
        if (mEnableColorTest) {
            if (mEnableColorPoint) {
                mFilterPoint = new FilterRGBPoint();
                mFilterPoint->init();
            } else {
                mProgramID              =
                        loadProgram(RgbaVertexShaderCode.c_str(), hackFragmentShaderCode.c_str());
                mUniformColorLoc        = glGetUniformLocation(mProgramID, "color");
                mUniformWidthLoc        = glGetUniformLocation(mProgramID, "width");
                mUniformHeightLoc       = glGetUniformLocation(mProgramID, "height");
                mUniformTexLoc          = glGetUniformLocation(mProgramID, "SamplerRGBA");
                mAttribPosLocation      = glGetAttribLocation(mProgramID, "attPosition");
                mAttribTexCoordLocation = glGetAttribLocation(mProgramID, "attTexCoord");
            }
        } else {
            mProgramID              =
                    loadProgram(RgbaVertexShaderCode.c_str(), RgbaFragmentShaderCode.c_str());
            mUniformTexLoc          = glGetUniformLocation(mProgramID, "SamplerRGBA");
            mAttribPosLocation      = glGetAttribLocation(mProgramID, "attPosition");
            mAttribTexCoordLocation = glGetAttribLocation(mProgramID, "attTexCoord");
        }
    }

    void FilterRGB::onInitialized() {
        if (mEnableColorTest) {
            if (mEnableColorPoint) {
                mIsInitialized = mFilterPoint->isInitialized();
                return;
            }

            if (mUniformColorLoc < 0 || mUniformWidthLoc < 0 || mUniformHeightLoc < 0) {
                mIsInitialized = false;
            }
        }

        if (mProgramID == GL_NONE || mUniformTexLoc < 0 || mAttribPosLocation < 0 ||
            mAttribTexCoordLocation < 0) {
            mIsInitialized = false;
        }

        if (!mIsInitialized) {
            LOGE("[FilterRGB] init failed.");
        }
    }

    void FilterRGB::draw(const GLuint *textureId) {
        if (mEnableColorTest && mEnableColorPoint) {
            drawWithColor();
            return;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId[0]);
        glUniform1i(mUniformTexLoc, 0);
        if (mEnableColorTest) {
            switchColor();
            glUniform1f(mUniformWidthLoc, mWidth * 1.0f);
            glUniform1f(mUniformHeightLoc, mHeight * 1.0f);
            glUniform3fv(mUniformColorLoc, 1, mColorTest);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void FilterRGB::drawWithColor() {
        switchColor();
        FilterRGBPoint::Vec3 color(mColorTest[0], mColorTest[1], mColorTest[2]);
        mFilterPoint->draw(color);
    }

    void FilterRGB::setOutputSize(int outputWidth, int outputHeight) {
        mWidth  = outputWidth;
        mHeight = outputHeight;
    }

    void FilterRGB::switchColor() {
        mColorTest[0] = mColorSwitch ? 1.0f : 0.0f;
        mColorTest[1] = mColorSwitch ? 0.0f : 1.0f;
        mColorTest[2] = 0.0f;
        mColorSwitch = !mColorSwitch;
    }
}  // namespace pipeline
