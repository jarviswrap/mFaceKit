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

        void resize(int width, int height) override;
        Error render(const std::shared_ptr<RenderData<PixelData>> &data) override;
        void destroy() override;

    private:
        void initGL(PixelFormat format);
        void checkGlError(const char* op);
        void resetTextureSwizzle(GLenum target);
        GLuint loadShader(GLenum type, const char* shaderCode);
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);
        void updateTextures(const std::shared_ptr<PixelData>& data);

        bool mInitialized{false};
        int mWidth{0};
        int mHeight{0};

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

//        std::shared_ptr<BBoxRender> mBBoxRender{nullptr};
//        std::shared_ptr<FaceMeshRender> mFaceRender{nullptr};
//        std::shared_ptr<DelaunayDebugRender> mDelaunayRender{nullptr};
        // FBO for Off-screen rendering (Face Lift)
//        GLuint mFBO{0};
//        GLuint mFBOTexture{0};
//        int mFBOWidth{0};
//        int mFBOHeight{0};
        
//        void initFBO(int width, int height);
//        void destroyFBO();
    };

} // face

#endif //FACEDEMO_PIXELRENDER_HPP
