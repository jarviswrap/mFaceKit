#pragma once

#include "render/utils/GLType.hpp"
#include <string>

namespace face {
    const static float CUBE[] = {
            -1.0f,
            -1.0f,
            1.0f,
            -1.0f,
            -1.0f,
            1.0f,
            1.0f,
            1.0f,
    };

    const static float TEXTURE_COORD[] = {
            0.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
    };

    class FilterBase {
    public:
        FilterBase();
        virtual ~FilterBase();

        virtual void init();

        virtual void onInit();

        virtual void onInitialized();
        bool isInitialized();

        void setTexCoords(const float *coords);
        void setVertice(const float *vertice);

        void useProgram();
        void unUseProgram();

        virtual void draw(const GLuint *textureId);

        int loadProgram(const char *vShaderStr, const char *fShaderStr);

    protected:
        bool   mIsInitialized{false};
        GLuint mProgramID{0};
        GLint  mAttribPosLocation{-1};
        GLint  mAttribTexCoordLocation{-1};

    private:
        float mGLCubeBuffer[8]{0};
        float mGLTextureBuffer[8]{0};
    };
}  // namespace pipeline