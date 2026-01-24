//
// Created by wilbert on 2026/1/21.
//

#include "PixelRender.hpp"
#include "common/Log.hpp"
#include <vector>

namespace face {

    static const char* VERTEX_SHADER = R"(#version 300 es
    layout(location = 0) in vec4 aPosition;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 vTexCoord;
    void main() {
        gl_Position = aPosition;
        vTexCoord = aTexCoord;
    }
    )";

    static const char* FRAGMENT_SHADER_I420 = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    uniform sampler2D yTexture;
    uniform sampler2D uTexture;
    uniform sampler2D vTexture;
    void main() {
        float y = texture(yTexture, vTexCoord).r;
        float u = texture(uTexture, vTexCoord).r - 0.5;
        float v = texture(vTexture, vTexCoord).r - 0.5;
        float r = y + 1.402 * v;
        float g = y - 0.34414 * u - 0.71414 * v;
        float b = y + 1.772 * u;
        fragColor = vec4(r, g, b, 1.0);
    }
    )";

    static const char* FRAGMENT_SHADER_NV21 = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    uniform sampler2D yTexture;
    uniform sampler2D uvTexture;
    void main() {
        float y = texture(yTexture, vTexCoord).r;
        vec2 uv = texture(uvTexture, vTexCoord).rg - vec2(0.5);
        float v = uv.r;
        float u = uv.g;
        float r = y + 1.402 * v;
        float g = y - 0.34414 * u - 0.71414 * v;
        float b = y + 1.772 * u;
        fragColor = vec4(r, g, b, 1.0);
    }
    )";

    static const char* FRAGMENT_SHADER_RGB = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    uniform sampler2D rgbTexture;
    void main() {
        fragColor = texture(rgbTexture, vTexCoord);
    }
    )";

    static float VERTICES[] = {
            // positions        // texture coords
            -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, // Top-left
            -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, // Bottom-left
            1.0f,  1.0f, 0.0f,  1.0f, 0.0f, // Top-right
            1.0f, -1.0f, 0.0f,  1.0f, 1.0f  // Bottom-right
    };

    void PixelRender::setScaleType(ScaleType type) {
        mScaleType = type;
    }

    void PixelRender::onDestroy() {
        LOGE("PixelRender::%s mInitialized:%d", __FUNCTION__, mInitialized);
        if (mInitialized) {
            if (mVBO) glDeleteBuffers(1, &mVBO);
            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mTextureCount > 0) glDeleteTextures(mTextureCount, mTextures);
            mTextureCount = 0;
            if (mProgram) glDeleteProgram(mProgram);
            mInitialized = false;
        }
        if (mBBoxRender) {
            mBBoxRender->onDestroy();
            mBBoxRender = nullptr;
        }
    }

    void PixelRender::onSurfaceChanged(int width, int height) {
        mWidth = width;
        mHeight = height;
        glViewport(0, 0, width, height);
    }

    void PixelRender::initGL(PixelFormat format) {
        if (!mInitialized) {
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
            LOGE("PixelRender::%s mVAO:%d, mVBO:%d", __FUNCTION__, mVAO, mVBO);
            mInitialized = true;
        }

        if (mProgram != 0 && mPixelFormat == format) {
            return;
        }
        if (mProgram != 0) {
            LOGE("PixelRender::%s delete oldProgram:%d", __FUNCTION__, mProgram);
            glDeleteProgram(mProgram);
            mProgram = 0;
        }

        if (mTextureCount > 0) {
            glDeleteTextures(mTextureCount, mTextures);
            memset(mTextures, 0, sizeof(mTextures));
            mTextureCount = 0;
        }

        int neededTextures = 1;
        if (format == PixelFormat::I420P) neededTextures = 3;
        else if (format == PixelFormat::NV21) neededTextures = 2;

        glGenTextures(neededTextures, mTextures);
        mTextureCount = neededTextures;

        for(int i = 0; i < mTextureCount; i++) {
            glBindTexture(GL_TEXTURE_2D, mTextures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }


        LOGE("PixelRender::%s pixelFormat changed: %d => %d, mTextureCount:%d", __FUNCTION__, mPixelFormat, format, mTextureCount);
        mPixelFormat = format;
        switch (format) {
            case PixelFormat::I420P:
                mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER_I420);
                mUniformsI420.textureY = glGetUniformLocation(mProgram, "yTexture");
                mUniformsI420.textureU = glGetUniformLocation(mProgram, "uTexture");
                mUniformsI420.textureV = glGetUniformLocation(mProgram, "vTexture");
                LOGE("PixelRender::%s mProgram:%d UniformLocation{yTexture:%d, uTexture:%d, vTexture:%d}", __FUNCTION__, mProgram, mUniformsI420.textureY, mUniformsI420.textureU, mUniformsI420.textureV);
                break;
            case PixelFormat::NV21:
                mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER_NV21);
                mUniformsNV21.textureY = glGetUniformLocation(mProgram, "yTexture");
                mUniformsNV21.textureUV = glGetUniformLocation(mProgram, "uvTexture");
                LOGE("PixelRender::%s mProgram:%d UniformLocation{yTexture:%d, uvTexture:%d}", __FUNCTION__, mProgram, mUniformsI420.textureY, mUniformsNV21.textureUV);
                break;
            case PixelFormat::RGB:
            case PixelFormat::BGR:
            case PixelFormat::RGBA:
            case PixelFormat::ARGB:
                mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER_RGB);
                mUniformsRGB.textureRGB = glGetUniformLocation(mProgram, "rgbTexture");
                LOGE("PixelRender::%s mProgram:%d UniformLocation{rgbTexture:%d}", __FUNCTION__, mProgram, mUniformsRGB.textureRGB);
                break;
            default:
                LOGE("PixelRender::%s format:%d not handled", __FUNCTION__, format);
                break;
        }
    }

    Error PixelRender::onDrawFrame(const std::shared_ptr<face::PixelData> &data) {
        if (!data || data->isEmpty()) {
            LOGE("PixelRender::%s data[%p] invalid", __FUNCTION__, data.get());
            return Error::Err_InvalidInput;
        }
        if (mWidth == 0 || mHeight == 0) {
            LOGE("PixelRender::%s surfaceSizeError:%dx%d", __FUNCTION__, mWidth, mHeight);
            return Error::Err_InvalidSurface;
        }
        
        // 1. Convert YUV/RGB to RGB Texture (mTextures[0] or intermediate)
        // We will render to FBO to get a clean RGB texture for FaceLift
        
        initGL(data->getFormat());
        int imageWidth = data->getResolution().getWidth();
        int imageHeight = data->getResolution().getHeight();
        
        initFBO(imageWidth, imageHeight);
        
        // Render current frame to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glViewport(0, 0, imageWidth, imageHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Use full screen quad for FBO rendering (scale 1.0)
        // We need a separate VBO/VAO for full screen or just modify VERTICES temporarily?
        // Better to use a standard full screen quad
        // But reusing current updateTextures logic which depends on mProgram
        
        if (mProgram) {
            updateTextures(data);
            
            // Draw full screen quad to FBO (no cropping yet)
            // Use simple vertices [-1, -1] to [1, 1]
            float fullQuad[] = {
                -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
                 1.0f, -1.0f, 0.0f,  1.0f, 1.0f
            };
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fullQuad), fullQuad);
            glBindVertexArray(mVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 2. Apply Face Lift (FBO Texture -> Screen or Back to FBO?)
        // Render directly to Screen with scaling
        glViewport(0, 0, mWidth, mHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if (!mFaceRender) {
            mFaceRender = std::make_shared<FaceMeshRender>();
        }
        mFaceRender->init();
        mFaceRender->setViewSize(mWidth, mHeight);
        mFaceRender->setIntensity(data->faceListIntensity);

        if (!mDelaunayRender) {
            mDelaunayRender = std::make_shared<DelaunayDebugRender>();
        }
        mDelaunayRender->init();
        mDelaunayRender->setViewSize(mWidth, mHeight);
        // Calculate Viewport based on ScaleType
        // Similar to updateVertex logic but applying to Viewport instead of Vertices
        // FaceMeshRender draws a full screen quad [-1, 1], so Viewport controls the placement
        
        int vw = mWidth;
        int vh = mHeight;
        int x = 0;
        int y = 0;
        
        float viewAspect = (float)mWidth / mHeight;
        float imageAspect = (float)imageWidth / imageHeight;
        
        if (mScaleType == ScaleType::FitCenter) {
             if (imageAspect > viewAspect) {
                 // Image is wider, fit width, black bars top/bottom
                 // vw = mWidth;
                 vh = (int)(mWidth / imageAspect);
                 y = (mHeight - vh) / 2;
             } else {
                 // Image is taller, fit height, black bars left/right
                 // vh = mHeight;
                 vw = (int)(mHeight * imageAspect);
                 x = (mWidth - vw) / 2;
             }
        } else if (mScaleType == ScaleType::CenterCrop) {
             if (imageAspect > viewAspect) {
                 // Image is wider, crop width -> fill height
                 // vh = mHeight;
                 vw = (int)(mHeight * imageAspect);
                 x = (mWidth - vw) / 2; // x will be negative
             } else {
                 // Image is taller, crop height -> fill width
                 // vw = mWidth;
                 vh = (int)(mWidth / imageAspect);
                 y = (mHeight - vh) / 2; // y will be negative
             }
        }
        // FitXY: use full mWidth, mHeight (default)
        
        glViewport(x, y, vw, vh);
        


        if (!data->bboxes.empty()) {
            mFaceRender->draw(mFBOTexture, data->bboxes, imageWidth, imageHeight);
             if (!mBBoxRender) {
                 mBBoxRender = std::make_shared<BBoxRender>();
             }
             mBBoxRender->init();
             // BBoxRender draws in NDC [-1, 1] relative to the current Viewport
             // Since we set Viewport to match the image area, NDC maps correctly to image coordinates
             mBBoxRender->draw(data->bboxes, vw, vh, imageWidth, imageHeight, 1.0f, 1.0f);
             mDelaunayRender->draw(data->bboxes, imageWidth, imageHeight);
        }

        return Error::None;
    }

    void PixelRender::initFBO(int width, int height) {
        if (mFBO != 0 && mFBOWidth == width && mFBOHeight == height) {
            return;
        }
        destroyFBO();
        
        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        
        glGenTextures(1, &mFBOTexture);
        glBindTexture(GL_TEXTURE_2D, mFBOTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFBOTexture, 0);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOGE("PixelRender::initFBO failed");
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mFBOWidth = width;
        mFBOHeight = height;
    }

    void PixelRender::destroyFBO() {
        if (mFBOTexture) {
            glDeleteTextures(1, &mFBOTexture);
            mFBOTexture = 0;
        }
        if (mFBO) {
            glDeleteFramebuffers(1, &mFBO);
            mFBO = 0;
        }
        mFBOWidth = 0;
        mFBOHeight = 0;
    }

    void PixelRender::updateTextures(const std::shared_ptr<PixelData>& data) {
        int width = data->getResolution().getWidth();
        int height = data->getResolution().getHeight();
        const uint8_t* pixels = data->getPixels();
        
        glUseProgram(mProgram);

        switch(data->getFormat()) {
            case PixelFormat::I420P: {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // Y
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTextures[0]);
                resetTextureSwizzle(GL_TEXTURE_2D);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
                glUniform1i(mUniformsI420.textureY, 0);

                // U
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mTextures[1]);
                resetTextureSwizzle(GL_TEXTURE_2D);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, pixels + width * height);
                glUniform1i(mUniformsI420.textureU, 1);

                // V
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mTextures[2]);
                resetTextureSwizzle(GL_TEXTURE_2D);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, pixels + width * height * 5 / 4);
                glUniform1i(mUniformsI420.textureV, 2);
                break;
            }
            case PixelFormat::NV21: {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // Y
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTextures[0]);
                resetTextureSwizzle(GL_TEXTURE_2D);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
                glUniform1i(mUniformsNV21.textureY, 0);

                // UV (Interleaved V U)
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mTextures[1]);
                resetTextureSwizzle(GL_TEXTURE_2D);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width/2, height/2, 0, GL_RG, GL_UNSIGNED_BYTE, pixels + width * height);
                glUniform1i(mUniformsNV21.textureUV, 1);
                break;
            }
            case PixelFormat::RGB:
            case PixelFormat::BGR:
            case PixelFormat::RGBA:
            case PixelFormat::ARGB: {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTextures[0]);
                resetTextureSwizzle(GL_TEXTURE_2D);

                GLint internalFormat = GL_RGBA;
                GLenum format = GL_RGBA;

                if (data->getFormat() == PixelFormat::RGB) {
                    internalFormat = GL_RGB;
                    format = GL_RGB;
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                } else if (data->getFormat() == PixelFormat::BGR) {
                    internalFormat = GL_RGB;
                    format = GL_RGB;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                } else if (data->getFormat() == PixelFormat::RGBA) {
                    internalFormat = GL_RGBA;
                    format = GL_RGBA;
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                } else if (data->getFormat() == PixelFormat::ARGB) {
                    internalFormat = GL_RGBA;
                    format = GL_RGBA;
                    // Assuming ARGB bytes are A R G B
                    // R=A, G=R, B=G, A=B
                    // Map to: R=Green(R), G=Blue(G), B=Alpha(B), A=Red(A)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ALPHA);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                }
                LOGE("PixelRender::%s internalFormat:%d, width:%d, height:%d, format:%d, pixels:%p", __FUNCTION__, internalFormat, width, height, format, pixels);
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
                glUniform1i(mUniformsRGB.textureRGB, 0);
                break;
            }
            case PixelFormat::UNKNOWN:break;
        }
    }

    void PixelRender::resetTextureSwizzle(GLenum target) {
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
    }

    void PixelRender::checkGlError(const char* op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            LOGE("after %s() glError (0x%x)\n", op, error);
        }
    }

    GLuint PixelRender::loadShader(GLenum type, const char* shaderCode) {
        GLuint shader = glCreateShader(type);
        if (shader == 0) {
            return 0;
        }
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char* infoLog = (char*)malloc(sizeof(char) * infoLen);
                glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                LOGE("Error compiling shader:\n%s\n", infoLog);
                free(infoLog);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint PixelRender::createProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertexShader) {
            return 0;
        }
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (!fragmentShader) {
            return 0;
        }
        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            glAttachShader(program, fragmentShader);
            checkGlError("glAttachShader");
            glLinkProgram(program);
            GLint linkStatus;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (!linkStatus) {
                GLint infoLen = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen > 1) {
                    char* infoLog = (char*)malloc(sizeof(char) * infoLen);
                    glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
                    LOGE("Error linking program:\n%s\n", infoLog);
                    free(infoLog);
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

} // face