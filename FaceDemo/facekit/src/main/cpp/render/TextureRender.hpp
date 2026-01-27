//
// Created by wilbert on 2026/1/27.
//

#ifndef FACEDEMO_TEXTURERENDER_HPP
#define FACEDEMO_TEXTURERENDER_HPP
//#include "common/RenderData.hpp"
#include "render/Render.hpp"
#include <GLES3/gl3.h>

namespace face {

    class Texture;
    class TextureRender : public Render<Texture> {
    public:
        void resize(int width, int height) override;
        Error render(const std::shared_ptr<RenderData<Texture>> &data) override;
        void destroy() override;

    private:
        void initGL();
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);
        GLuint loadShader(GLenum type, const char* shaderCode);
        void checkGlError(const char* op);

        bool mInitialized{false};
        int mWidth{0};
        int mHeight{0};

        GLuint mProgram{0};
        GLuint mVAO{0};
        GLuint mVBO{0};
        GLint mTextureLocation{-1};
        GLint mMVPLocation{-1};
    };

} // face

#endif //FACEDEMO_TEXTURERENDER_HPP
