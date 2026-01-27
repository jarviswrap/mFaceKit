//
// Created by wilbert on 2022/7/25.
//

#pragma once

#include "render/utils/GLType.hpp"

namespace face {
    class Texture {
    public:
        Texture() : mTexId(0), mWidth(0), mHeight(0) {};
        ~Texture();
        bool init(int w, int h, const uint8_t *data);
        bool release();
        void reset();
        bool replace(int width, int height, GLuint textureId);

        inline bool initialized() { return mTexId > 0; };

        inline GLuint get() const { return mTexId; };

        inline int width() { return mWidth; };

        inline int height() { return mHeight; };

        inline bool isSameSize(int width, int height) {
            return mWidth == width && mHeight == height;
        };
    private:
        GLuint mTexId{0};
        int    mWidth{0};
        int    mHeight{0};
        bool   mIsOuterTex{false};
    };
}

