#include "OpenGLUtils.hpp"

#include "common/Log.hpp"
#include <string>
#include <sstream>
#include <malloc.h>
#include "FrameBuffer.hpp"

namespace face
{

constexpr std::array<float, 8> OpenGLUtils::DEFAULT_VBO_POS;
constexpr std::array<float, 8> OpenGLUtils::DEFAULT_VBO_TEX_COORD;

GLuint OpenGLUtils::loadProgram(const char* vShaderStr, const char* fShaderStr)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vShaderStr);
    if (vertexShader == 0)
    {
        LOGE("[%s] Vertex Shader Failed", __FUNCTION__);
        return 0;
    }
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fShaderStr);
    if (fragmentShader == 0)
    {
        LOGE("[%s] Fragment Shader Failed", __FUNCTION__);
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint programID = glCreateProgram();
    if (programID == 0)
    {
        return 0;
    }

    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);

    GLint linked;
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        int infoLen = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char* infoLog = ( char* )malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programID, infoLen, NULL, infoLog);
            LOGE("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteProgram(programID);
        programID = GL_FALSE;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return programID;
}

GLuint OpenGLUtils::loadShader(GLenum type, const char* shaderSrc)
{
    GLint  compiled;
    GLuint shader = glCreateShader(type);
    if (shader == 0)
    {
        LOGE("OpenGLUtils glCreateShader = 0, ");
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            char* infoLog = ( char* )malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("Error compiling shader:\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool OpenGLUtils::CreateTextures(GLuint* textures, int num, int width, int height, bool isYuv)
{
    if (num <= 0 || width <= 0 || height <= 0)
    {
        return false;
    }
    glGenTextures(num, textures);
    bool genFail = false;
    for (uint32_t i = 0; i < num; i++)
    {
        if (textures[i] == 0)
        {
            LOGE("OpenGLUtils [createTextures] fail for %d", i);
            genFail = true;
            break;
        }
    }
    if (genFail)
    {
        LOGE("OpenGLUtils [createTextures] glGenTextures glGetError=%d", glGetError());
        DeleteTextures(textures, num);
        return false;
    }
    for (uint32_t i = 0; i < num; i++)
    {
        GLuint texture = textures[i];
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture);
        if (isYuv)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, GL_TRUE);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        int    widthTemp  = width;
        int    heightTemp = height;
        GLenum format     = GL_RGBA;
        if (isYuv)
        {
            widthTemp  = i == 0 ? width : width / 2;
            heightTemp = i == 0 ? height : height / 2;
            format     = GL_LUMINANCE;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, widthTemp, heightTemp, 0, format, GL_UNSIGNED_BYTE, nullptr);
    }
    return true;
}

void OpenGLUtils::DeleteTextures(GLuint* textures, int num)
{
    glDeleteTextures(num, textures);
}

void OpenGLUtils::YuvDataToTextures(uint8_t* yuvData, int width, int height, GLuint* textures)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    uint8_t* yDataPointer = yuvData;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yDataPointer);

    uint8_t* uDataPointer = yuvData + width * height;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, uDataPointer);

    uint8_t* vDataPointer = yuvData + width * height * 5 / 4;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, vDataPointer);
}

    GLuint OpenGLUtils::CreateTexture(int width, int height, const uint8_t* data)
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        if (texture == 0)
        {
            LOGE("OpenGLUtils [createTextures] glGenTextures glGetError=%d", glGetError());
            return 0;
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        return texture;
    }

    void OpenGLUtils::ClearGLState()
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);  // 引擎释放素材可能会关闭了color write
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void OpenGLUtils::CheckGLErrors(const char* tag)
    {
        GLenum glerror;
        while ((glerror = glGetError()) != GL_NO_ERROR)
        {
            std::stringstream sstream;
            sstream << tag << ":" << glerror;

            // LogLimiter使用format作为key来限制打印，这里拼接避免漏掉打印
            std::string msg = "glerror=" + sstream.str();
            LOGE("%s", msg.c_str());
        }
    }

#ifdef __ANDROID__
    GLuint OpenGLUtils::createOESTextureID()
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return texture;
    }
#endif

    GLuint OpenGLUtils::createFrameBuffer()
    {
        GLuint values;
        glGenFramebuffers(1, &values);
        glBindFramebuffer(GL_FRAMEBUFFER, values);
        return values;
    }

    void OpenGLUtils::deleteFrameBuffer(GLuint fboId)
    {
        GLuint fbos = fboId;
        glDeleteFramebuffers(1, &fbos);
    }

    bool OpenGLUtils::attachmentTexture(GLuint textureId)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LOGE("OpenGLUtils [FrameBuffer] init error. status = %d , textureId=%d", status, textureId);
        }
        return true;
    }

    void OpenGLUtils::viewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        glViewport(x, y, width, height);
    }

    GLint OpenGLUtils::getCurrentFbo()
    {
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        return fbo;
    }

    bool OpenGLUtils::bindFrameBuffer(GLuint fboId)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        return true;
    }

    void OpenGLUtils::readPixel(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
    {
        if (!pixels || width <= 0 || height <= 0)
        {
            LOGE("OpenGLUtils %s failed, pixels:%p, [%d,%d]", __FUNCTION__, pixels, width, height);
            return;
        }
        //宽字节对齐,默认的对齐字节为4，如果宽度未对齐(如宽度为14)，glReadPixels会崩溃(fault addr)
        //RGB才需要对齐，RGBA本身就是4字节对齐，不需要设置
        uint32_t align    = 0;
        GLint    defAlign = 4;
        if (format == GL_RGB)
        {
            align = width % 4;
        }
        if (align != 0)
        {
            glGetIntegerv(GL_PACK_ALIGNMENT, &defAlign);  //获取设置之前的对齐值
            glPixelStorei(GL_PACK_ALIGNMENT, align == 2 ? 2 : 1);
        }
        glReadPixels(0, 0, width, height, format, type, pixels);
        if (align != 0)  //恢复之前的字节对齐
        {
            glPixelStorei(GL_PACK_ALIGNMENT, defAlign);
        }
    }

    void OpenGLUtils::flush()
    {
        glFlush();
    }

    void  OpenGLUtils::finish()
    {
        glFinish();
    }

    void OpenGLUtils::initDefaultVBO(GLuint& posVBOId, GLuint& texVBOId)
    {
        //create VBOs
        glGenBuffers(1, &posVBOId);
        glBindBuffer(GL_ARRAY_BUFFER, posVBOId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * DEFAULT_VBO_POS.size(), DEFAULT_VBO_POS.data(), GL_STATIC_DRAW);
        glGenBuffers(1, &texVBOId);
        glBindBuffer(GL_ARRAY_BUFFER, texVBOId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * DEFAULT_VBO_TEX_COORD.size(), DEFAULT_VBO_TEX_COORD.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint OpenGLUtils::createTexture()
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return texture;
    }
}  // namespace pipeline
