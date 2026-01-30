//
// Created by wilbert on 2026/1/21.
//

#ifndef FACEDEMO_PIXELRENDER_HPP
#define FACEDEMO_PIXELRENDER_HPP
#include "Render.hpp"
#include "common/PixelData.hpp"
#include <GLES3/gl3.h>
#include "BBoxRender.hpp"
#include "FaceMeshRender.hpp"
#include "DelaunayDebugRender.hpp"

namespace face {

    class PixelRender : public Render<PixelData>{

    public:
        PixelRender();
        ~PixelRender() override;
    protected:
        bool getDataSize(const std::shared_ptr<PixelData> &data, int &width, int &height) override;
        Error onRender(const std::shared_ptr<face::PixelData> &data, int rotation) override;
        void onDestroy() override;

    private:
        void initGL(PixelFormat format);
        void deleteProgram();
        void deleteTextures();
        void resetTextureSwizzle(GLenum target);
        void updateTextures(const std::shared_ptr<PixelData>& data);

        bool mInitialized{false};

        // OpenGL resources
        GLuint mProgram{0};
        PixelFormat mPixelFormat{PixelFormat::UNKNOWN};
        
        GLuint mVBO{0};
        GLuint mVAO{0};
        
        // Textures
        GLuint mTextures[3]{0}; // Y, U, V or RGB
        int mTextureCount{0}; // Actual number of textures in use
        
        // Uniform locations
        struct {
            GLint textureY{-1};
            GLint textureU{-1};
            GLint textureV{-1};
        } mUniformsI420;
        
        struct {
            GLint textureY{-1};
            GLint textureUV{-1};
        } mUniformsNV21;
        
        struct {
            GLint textureRGB{-1};
        } mUniformsRGB;
    };

} // face

#endif //FACEDEMO_PIXELRENDER_HPP
