//
// Created by wilbert on 2022/7/25.
//

#include "Texture.hpp"
#include "common/Log.hpp"
#include "render/utils/OpenGLUtils.hpp"

namespace face {
    Texture::~Texture() {
        release();
    }

    bool Texture::init(int w, int h, const uint8_t *data) {
        if (mTexId > 0 && w == mWidth && h == mHeight) {
            return false;
        }
        release();
        mTexId      = OpenGLUtils::CreateTexture(w, h, data);
        mWidth      = w;
        mHeight     = h;
        mIsOuterTex = false;

        return mTexId > 0;
    }

    bool Texture::replace(int width, int height, GLuint textureId) {
        mIsOuterTex = mTexId != textureId;
        if (mIsOuterTex && mTexId > 0) {
            //是外部纹理，先删除内部纹理
            OpenGLUtils::DeleteTextures(&mTexId, 1);
        }

        mTexId  = textureId;
        mWidth  = width;
        mHeight = height;
        return true;
    }

    bool Texture::release() {

        LOGI("[FrameBuffer] Texture::release %d, size(%d, %d)", mTexId, mWidth, mHeight);
        if (mTexId == 0) {
            return false;
        }
        if (!mIsOuterTex) {
            OpenGLUtils::DeleteTextures(&mTexId, 1);
        }
        mTexId  = 0;
        mWidth  = 0;
        mHeight = 0;
        return true;
    }

    void Texture::reset() {
        mTexId  = 0;
        mWidth  = 0;
        mHeight = 0;
    }
}  // namespace pipeline
