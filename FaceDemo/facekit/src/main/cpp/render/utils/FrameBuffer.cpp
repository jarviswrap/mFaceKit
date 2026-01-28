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
    , mIsInited(false)
{
}

FrameBuffer::~FrameBuffer()
{
    release();
}

bool FrameBuffer::init(int width, int height)
{
    return init(width, height, false);
}

bool FrameBuffer::init(int width, int height, bool depth)
{
    LOGE("FrameBuffer::%s %dx%d", __FUNCTION__, width, height);
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

    GLuint values;
    glGenFramebuffers(1, &values);
    mFbo = values;

    if (depth)
    {
        mDepthBuffer = _createRenderBuffer(width, height);
    }

    GLint curFbo = getCurrentFbo();

    glBindFramebuffer(GL_FRAMEBUFFER, ( GLuint )mFbo);
    if (!mTexture->getTextureId()) {
        LOGE("FrameBuffer::%s failed, mTexture invalid", __FUNCTION__);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture->getTextureId(), 0);
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
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
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
