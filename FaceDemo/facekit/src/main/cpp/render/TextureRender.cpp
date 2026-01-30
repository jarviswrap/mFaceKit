//
// Created by wilbert on 2026/1/27.
//

#include <malloc.h>
#include "TextureRender.hpp"
#include "common/Log.hpp"
#include "render/utils/Matrix.hpp"
#include "render/utils/OpenGLUtils.hpp"

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

    Error TextureRender::onRender(const std::shared_ptr<Texture> &texture, int rotation) {
        if (!texture || !texture->getTextureId()) {
            return Error::Err_InvalidInput;
        }
        initGL();
        if (!mInitialized) {
            return Error::Err_OpenGLError;
        }
        LOGE("TextureRender::%s, data:%p, rotation:%d, textureId:[%d, %dx%d]", __FUNCTION__, texture.get(), rotation, texture->getTextureId(), texture->width(), texture->height());
        glUseProgram(mProgram);
        OpenGLUtils::checkGLErrors("glUseProgram");

        glBindVertexArray(mVAO);
        OpenGLUtils::checkGLErrors("glBindVertexArray");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
        glUniform1i(mTextureLocation, 0);

        Matrix mvp;
        mvp.postRotate(0, 0.0f, 0.0f);
        glUniformMatrix4fv(mMVPLocation, 1, GL_FALSE, mvp.peek());

        // Calculate Viewport based on ScaleType
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        OpenGLUtils::checkGLErrors("glDrawArrays");

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        return Error::None;
    }

    void TextureRender::onDestroy() {
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

    bool TextureRender::getDataSize(const std::shared_ptr<face::Texture> &data,
                                    int &width,
                                    int &height) {
        if(data) {
            width = data->width();
            height = data->height();
            return true;
        }
        return false;
    }

    void TextureRender::initGL() {
        if (mInitialized) return;

        mProgram = OpenGLUtils::loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
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

} // face
