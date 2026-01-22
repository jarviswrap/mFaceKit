//
// Created by wilbert on 2026/1/21.
//

#ifndef FACEDEMO_PIXELRENDER_HPP
#define FACEDEMO_PIXELRENDER_HPP
#include "Render.hpp"
#include "common/PixelData.hpp"
#include <GLES3/gl3.h>

namespace face {

    enum class ScaleType {
        FitXY,
        CenterCrop,
        FitCenter
    };

    class PixelRender : public Render<PixelData>{

    public:
        PixelRender() = default;
        ~PixelRender() override = default;

        void onSurfaceChanged(int width, int height) override;
        Error onDrawFrame(const std::shared_ptr<face::PixelData> &data) override;
        void onDestroy() override;
        void setScaleType(ScaleType type);

    private:
        void initGL(PixelFormat format);
        void checkGlError(const char* op);
        void resetTextureSwizzle(GLenum target);
        GLuint loadShader(GLenum type, const char* shaderCode);
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);
        void updateTextures(const std::shared_ptr<PixelData>& data);
        void updateVertex(int imageWidth, int imageHeight);

        bool mInitialized{false};
        int mWidth{0};
        int mHeight{0};
        ScaleType mScaleType{ScaleType::FitCenter};

        // Cache for updateVertex optimization
        int mLastImageWidth{0};
        int mLastImageHeight{0};
        ScaleType mLastScaleType{ScaleType::FitCenter};

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
