//
// Created by wilbert on 2026/1/27.
//

#ifndef FACEDEMO_TEXTURERENDER_HPP
#define FACEDEMO_TEXTURERENDER_HPP
#include "render/Render.hpp"
#include "Texture.hpp"
#include <GLES3/gl3.h>

namespace face {

    class TextureRender : public Render<Texture> {
    public:
        Error onRender(const std::shared_ptr<Texture> &data, int rotation) override;
        void onDestroy() override;
        bool getDataSize(const std::shared_ptr<face::Texture> &data, int &width, int &height) override;

    private:
        void initGL();

        bool mInitialized{false};

        GLuint mProgram{0};
        GLuint mVAO{0};
        GLuint mVBO{0};
        GLint mTextureLocation{-1};
        GLint mMVPLocation{-1};
    };

} // face

#endif //FACEDEMO_TEXTURERENDER_HPP
