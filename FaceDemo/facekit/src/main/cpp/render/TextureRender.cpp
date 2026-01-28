//
// Created by wilbert on 2026/1/27.
//

#include <malloc.h>
#include "TextureRender.hpp"
#include "common/Log.hpp"
#include "render/utils/Texture.hpp"
#include "render/utils/Matrix.hpp"

namespace face {

    static const char* VERTEX_SHADER = R"(#version 300 es
    layout(location = 0) in vec4 aPosition;
    layout(location = 1) in vec2 aTexCoord;
    uniform mat4 uMVPMatrix;
    out vec2 vTexCoord;
    void main() {
        gl_Position = uMVPMatrix * aPosition;
        vTexCoord = aTexCoord;
    }
    )";

    static const char* FRAGMENT_SHADER = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    uniform sampler2D sTexture;
    void main() {
        fragColor = texture(sTexture, vTexCoord);
    }
    )";

    static float VERTICES[] = {
            // positions        // texture coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // Top-right
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f  // Bottom-right
    };

    void TextureRender::resize(int width, int height) {
        LOGE("TextureRender::%s, %dx%d %dx%d", __FUNCTION__, mWidth, mHeight, width, height);
        mWidth = width;
        mHeight = height;
    }

    Error TextureRender::render(const std::shared_ptr<RenderData<Texture>> &data) {
        if (!data || !data->data) {
            return Error::Err_InvalidInput;
        }

        if (mWidth == 0 || mHeight == 0) {
            return Error::Err_InvalidSurface;
        }
        initGL();
        if (!mInitialized) {
            return Error::Err_OpenGLError;
        }
        auto& texture = data->data;
        LOGE("TextureRender::%s, data:%p, rotation:%f, textureId:%lu", __FUNCTION__, data.get(), data->rotation, (uint32_t)texture->getTextureId());
        glUseProgram(mProgram);
        checkGlError("glUseProgram");

        glBindVertexArray(mVAO);
        checkGlError("glBindVertexArray");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
        glUniform1i(mTextureLocation, 0);

        Matrix mvp;
        mvp.postRotate(data->rotation, 0.0f, 0.0f);
        glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, mvp.peek());

        // Calculate Viewport based on ScaleType
        int vw = mWidth;
        int vh = mHeight;
        int x = 0;
        int y = 0;

        int imageWidth = data->data->width();
        int imageHeight = data->data->height();

        if (imageWidth > 0 && imageHeight > 0) {
            float viewAspect = (float)mWidth / mHeight;
            float imageAspect = (float)imageWidth / imageHeight;

            if (data->scaleType == ScaleType::FitCenter) {
                if (imageAspect > viewAspect) {
                    // Image is wider, fit width
                    vh = (int)(mWidth / imageAspect);
                    y = (mHeight - vh) / 2;
                } else {
                    // Image is taller, fit height
                    vw = (int)(mHeight * imageAspect);
                    x = (mWidth - vw) / 2;
                }
            } else if (data->scaleType == ScaleType::CenterCrop) {
                if (imageAspect > viewAspect) {
                    // Image is wider, crop width -> fill height
                    vw = (int)(mHeight * imageAspect);
                    x = (mWidth - vw) / 2;
                } else {
                    // Image is taller, crop height -> fill width
                    vh = (int)(mWidth / imageAspect);
                    y = (mHeight - vh) / 2;
                }
            }
        }

        glViewport(x, y, vw, vh);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        checkGlError("glDrawArrays");

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        return Error::None;
    }

    void TextureRender::destroy() {
        LOGE("TextureRender::%s, mInitialized:%d", __FUNCTION__, mInitialized);
        if (mInitialized) {
            if (mProgram) {
                glDeleteProgram(mProgram);
                mProgram = 0;
            }
            if (mVAO) {
                glDeleteVertexArrays(1, &mVAO);
                mVAO = 0;
            }
            if (mVBO) {
                glDeleteBuffers(1, &mVBO);
                mVBO = 0;
            }
            mInitialized = false;
        }
    }

    void TextureRender::initGL() {
        if (mInitialized) return;

        mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
        LOGE("TextureRender::%s, mProgram:%d", __FUNCTION__, mProgram);
        if (!mProgram) {
            LOGE("TextureRender::initGL createProgram failed");
            return;
        }

        mTextureLocation = glGetUniformLocation(mProgram, "sTexture");
        mMVPLocation = glGetUniformLocation(mProgram, "uMVPMatrix");

        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        mInitialized = true;
        LOGI("TextureRender::initGL success");
    }

    GLuint TextureRender::createProgram(const char *vertexSource, const char *fragmentSource) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertexShader) return 0;

        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (!fragmentShader) return 0;

        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char* buf = (char*)malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        LOGE("TextureRender::createProgram Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    GLuint TextureRender::loadShader(GLenum type, const char *shaderCode) {
        GLuint shader = glCreateShader(type);
        if (shader) {
            glShaderSource(shader, 1, &shaderCode, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char* buf = (char*)malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        LOGE("TextureRender::loadShader Could not compile shader %d:\n%s\n", type, buf);
                        free(buf);
                    }
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }

    void TextureRender::checkGlError(const char *op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            LOGE("TextureRender::after %s() glError (0x%x)\n", op, error);
        }
    }

} // face
