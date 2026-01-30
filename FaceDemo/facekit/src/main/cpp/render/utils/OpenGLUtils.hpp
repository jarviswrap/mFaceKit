#pragma once
#include <memory>
#include <string>
#include <array>
#include "GLType.hpp"

namespace face
{
class FrameBuffer;
class OpenGLUtils
{
public:
    static GLuint loadProgram(const char* vShaderStr, const char* fShaderStr);
    static GLuint loadShader(GLenum type, const char* shaderSrc);
    static bool   createTextures(GLuint* textures, int num, int width, int height, bool isYuv);
    static void   deleteTextures(GLuint* textures, int num);
    static void   yuvDataToTextures(uint8_t* yuvData, int width, int height, GLuint* textures);

    static GLuint createTexture(int width, int height, const uint8_t* data);

    static void clearGLState();

    static void checkGLErrors(const char* tag);

#ifdef __ANDROID__
    static GLuint createOESTextureID();
#endif
    static GLuint createFrameBuffer();
    static void   deleteFrameBuffer(GLuint fboId);
    static bool   attachmentTexture(GLuint textureId);
    static void   viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    static GLint  getCurrentFbo();
    static bool   bindFrameBuffer(GLuint fboId);
    static void   readPixel(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);

    static void   flush();
    static void   initDefaultVBO(GLuint& posVBOId, GLuint& texVBOId);
    static GLuint createTexture();

    static void  finish();
public:
    static constexpr std::array<float, 8> DEFAULT_VBO_POS = {
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
    };

    static constexpr std::array<float, 8> DEFAULT_VBO_TEX_COORD = {
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
    };
};

}  // namespace pipeline
