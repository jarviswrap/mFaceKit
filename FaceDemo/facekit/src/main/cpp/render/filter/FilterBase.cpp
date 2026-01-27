#include "FilterBase.hpp"
#include "common/Log.hpp"
#include "render/utils/OpenGLUtils.hpp"

#include <string.h>
#include <cstring>

namespace face {
    FilterBase::FilterBase()
            : mIsInitialized(false), mProgramID(0) {
    }

    FilterBase::~FilterBase() {
        if (mProgramID > 0) {
            glDeleteProgram(mProgramID);
            mProgramID = 0;
        }
    }

    void FilterBase::init() {
        onInit();
        memcpy(mGLCubeBuffer, CUBE, 8 * sizeof(float));
        memcpy(mGLTextureBuffer, TEXTURE_COORD, 8 * sizeof(float));

        mIsInitialized = true;
        onInitialized();
    }

    void FilterBase::onInit() {
    }

    void FilterBase::onInitialized() {
    }

    void FilterBase::setTexCoords(const float *coords) {
        if (nullptr == coords) {
            LOGE("[FilterBase::%s] coords is nullptr", __FUNCTION__);
            return;
        }
        memcpy(mGLTextureBuffer, coords, 8 * sizeof(float));
    }

    void FilterBase::setVertice(const float *vertice) {
        if (nullptr == vertice) {
            LOGE("[FilterBase::%s] Vertice is nullptr", __FUNCTION__);
            return;
        }
        memcpy(mGLCubeBuffer, vertice, 8 * sizeof(float));
    }

    void FilterBase::useProgram() {
        glUseProgram(mProgramID);
        glVertexAttribPointer(mAttribPosLocation, 2, GL_FLOAT, false, 0, mGLCubeBuffer);
        glEnableVertexAttribArray(mAttribPosLocation);

        glVertexAttribPointer(mAttribTexCoordLocation, 2, GL_FLOAT, false, 0, mGLTextureBuffer);
        glEnableVertexAttribArray(mAttribTexCoordLocation);
    }

    void FilterBase::unUseProgram() {
        glDisableVertexAttribArray(mAttribPosLocation);
        glDisableVertexAttribArray(mAttribTexCoordLocation);
        glUseProgram(0);
    }

    void FilterBase::draw(const GLuint *textureId) {
    }

    bool FilterBase::isInitialized() {
        return mIsInitialized;
    }

    int FilterBase::loadProgram(const char *vShaderStr, const char *fShaderStr) {
        return OpenGLUtils::loadProgram(vShaderStr, fShaderStr);
    }

}  // namespace pipeline