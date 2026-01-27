#pragma once
#include "render/utils/GLType.hpp"
#include <memory>
namespace face
{
class FilterRGB;
class Texture;
class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    bool init(int width, int height);
    bool init(int width, int height, bool depth, int texId);
    bool isInitialized();

    int getWidth();
    int getHeight();

    bool bind();
    bool unbind();

    /**
     * 从texId画到本FBO绑定的纹理mTexId
     */
    bool drawWithTexture(int texId, bool enableColorTest = false, bool enableColorPoint = false);

    /**
     * 从texId画到本FBO绑定的纹理mTexId
     * @param texId 源纹理
     * @param x/y/width/height 视口坐标
     */
    bool drawWithTexture(int texId, int x, int y, int width, int height);

    bool readPixels(void* data, int size);

    GLint  getCurrentFbo();
    GLuint getFboId();
    std::shared_ptr<Texture> getFboTexture() const { return mTexture; };

    void release();

private:
    GLuint _createTexture(int width, int height);
    GLuint _createRenderBuffer(int width, int height);

private:
    std::shared_ptr<Texture>    mTexture;
    GLuint     mFbo{ 0 };
    GLuint     mPreFbo{ 0 };
//    GLuint     mTexId{ 0 };
    GLuint     mDepthBuffer{ 0 };
//    int        mWidth{ 0 };
//    int        mHeight{ 0 };
    bool       mIsInited{ false };
    bool       mIsBinded{ false };
    bool       mIsOuterTexture{ false };  //标记是否外部传入纹理，release时不做释放管理
    FilterRGB* mFilterRGB{ nullptr };
};
}  // namespace pipeline
