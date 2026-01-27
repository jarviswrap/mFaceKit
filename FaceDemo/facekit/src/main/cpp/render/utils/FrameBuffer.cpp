#include "FrameBuffer.hpp"
#include "common/Log.hpp"
#include "common/Common.hpp"
#include <string>
#include "render/utils/OpenGLUtils.hpp"
#include "render/filter/FilterRGB.hpp"
#include "render/utils/Texture.hpp"

namespace face
{
FrameBuffer::FrameBuffer()
    : mFbo(0)
    , mPreFbo(0)
    , mIsInited(false)
    , mIsBinded(false)
{
}

FrameBuffer::~FrameBuffer()
{
    release();
}

bool FrameBuffer::init(int width, int height)
{
    return init(width, height, false, 0);
}

bool FrameBuffer::init(int width, int height, bool depth, int texId)
{
    LOGE("[FrameBuffer] inited. width = %d, height = %d , texId: %d", width, height, texId);
    if (width <= 0 || height <= 0)
    {
        return false;
    }
    if (mIsInited)
    {
        return true;
    }

    mTexture = std::make_shared<Texture>();
    mTexture->init(width, height, nullptr);
//    mWidth    = width;
//    mHeight   = height;

    GLuint values;
    glGenFramebuffers(1, &values);
    mFbo = values;

    if (depth)
    {
        mDepthBuffer = _createRenderBuffer(width, height);
    }

    GLint curFbo = getCurrentFbo();

    mIsOuterTexture = texId > 0;
//    mTexId          = mIsOuterTexture ? texId : _createTexture(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, ( GLuint )mFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture->get(), 0);
    if (depth)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGE("[FrameBuffer] init error. status = %d ", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, curFbo);

    mIsInited = true;
    return mIsInited;
}

//bool FrameBuffer::attachTexture(int texId)
//{
//    //删除上次create的Texture
//    if (!mIsOuterTexture && mTexId > 0)
//    {
//        GLuint textures = mTexId;
//        glDeleteTextures(1, &textures);
//    }
//
//    OpenGLUtils::CheckGLErrors("FrameBuffer::attachTexture");
//
//    mTexId = texId;
//    bind();
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    if (status != GL_FRAMEBUFFER_COMPLETE)
//    {
//        LOGE("[FrameBuffer] xxx init error. status = %d  fbo-tex(%d, %d)", status, mFbo, mTexId);
//    }
//    unbind();
//    mIsOuterTexture = true;
//    return true;
//}

GLuint FrameBuffer::_createTexture(int width, int height)
{
    GLuint texture_map;
    glGenTextures(1, &texture_map);
    glBindTexture(GL_TEXTURE_2D, texture_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_map;
}

GLuint FrameBuffer::_createRenderBuffer(int width, int height)
{
    GLuint values;
    glGenRenderbuffers(1, &values);
    glBindRenderbuffer(GL_RENDERBUFFER, values);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    return values;
}

bool FrameBuffer::isInitialized()
{
    return mIsInited;
}

int FrameBuffer::getWidth()
{
    return mTexture->width();
}

int FrameBuffer::getHeight()
{
    return mTexture->height();
}

bool FrameBuffer::bind()
{
    if (!mIsInited)
    {
        LOGE("[FrameBuffer] not init");
        return false;
    }
    if (mIsBinded)
    {
        LOGE("[FrameBuffer] already binded. can not bind again");
        return true;
    }

    mPreFbo = getCurrentFbo();
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    mIsBinded = true;
    return true;
}

bool FrameBuffer::unbind()
{
    if (!mIsInited || !mIsBinded)
    {
        LOGE("[FrameBuffer] invalid status");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mPreFbo);
    mIsBinded = false;
    mPreFbo   = 0;
    return true;
}

bool FrameBuffer::drawWithTexture(int texId, bool enableColorTest, bool enableColorPoint)
{
    if (!mFilterRGB && enableColorTest)
    {
        mFilterRGB = new FilterRGB(enableColorTest, enableColorPoint);
        mFilterRGB->setOutputSize(mTexture->width(), mTexture->height());
    }
    return drawWithTexture(texId, 0, 0, mTexture->width(), mTexture->height());
}

bool FrameBuffer::drawWithTexture(int texId, int x, int y, int width, int height)
{
    if (!mFilterRGB)
    {
        mFilterRGB = new FilterRGB();
    }
    if (!mFilterRGB->isInitialized())
    {
        mFilterRGB->init();
    }
    if (!mFilterRGB->isInitialized())
    {
        SAFE_DELETE(mFilterRGB)
        LOGE("[FrameBuffer::%s] mFilterRGB init fail", __FUNCTION__);
        return false;
    }
    glViewport(x, y, width, height);
    mFilterRGB->useProgram();
    mFilterRGB->draw(reinterpret_cast<const GLuint*>(&texId));
    mFilterRGB->unUseProgram();
    return true;
}

bool FrameBuffer::readPixels(void* data, int size)
{
    if (!mTexture) return false;
    int pixelSize = 4;
    auto width = mTexture->width();
    auto height = mTexture->height();
    if (data == nullptr || size < width * height * pixelSize)
    {
        return false;
    }
    GLenum format = GL_RGBA;
    glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, ( GLvoid* )data);
    return true;
}

GLint FrameBuffer::getCurrentFbo()
{
    GLint curFbos;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curFbos);
    return curFbos;
}

GLuint FrameBuffer::getFboId()
{
    return mFbo;
}

void FrameBuffer::release()
{
    if (mIsInited)
    {
        mIsInited = false;
        if (mIsBinded && mPreFbo > 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mPreFbo);
        }
        if (mFbo != 0)
        {
            GLuint fbos = mFbo;
            glDeleteFramebuffers(1, &fbos);
            mFbo = 0;
        }

        if (mDepthBuffer != 0)
        {
            GLuint depthBuffer = mDepthBuffer;
            glDeleteRenderbuffers(1, &depthBuffer);
            mDepthBuffer = 0;
        }
        if (mTexture) {
            mTexture->release();
            mTexture.reset();
        }

        SAFE_DELETE(mFilterRGB)
    }
}
}  // namespace pipeline
