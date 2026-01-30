#pragma once
#include "render/utils/GLType.hpp"
#include "render/RenderTarget.hpp"

#include <memory>
namespace face
{
class FilterRGB;
class Texture;
class FrameBuffer: public RenderTarget{
public:
    FrameBuffer();
    ~FrameBuffer();

    bool init(int width, int height) override;
    int getWidth() override;
    int getHeight() override;
    bool bind() override;
    void release() override;

    bool init(int width, int height, bool depth);
    bool isInitialized();


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
private:
    GLuint _createRenderBuffer(int width, int height);

private:
    std::shared_ptr<Texture>    mTexture{nullptr};
    GLuint     mFbo{ 0 };
    GLuint     mDepthBuffer{ 0 };
    bool       mIsInited{ false };
    FilterRGB* mFilterRGB{ nullptr };
};
}  // namespace pipeline
