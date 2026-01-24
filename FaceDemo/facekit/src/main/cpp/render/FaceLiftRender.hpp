//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_FACELIFTRENDER_HPP
#define FACEDEMO_FACELIFTRENDER_HPP

#include <GLES3/gl3.h>
#include <vector>
#include "common/FaceBox.hpp"
#include "common/Size.hpp"

namespace face {

    class FaceLiftRender {
    public:
        FaceLiftRender();
        ~FaceLiftRender();

        void init();
        void setViewSize(int width, int height);
        void setIntensity(float intensity); // 0.0 - 1.0
        void draw(GLuint textureId, const std::vector<BBox>& bboxes, int imageWidth, int imageHeight);
        void onDestroy();

    private:
        GLuint loadShader(GLenum type, const char* shaderCode);
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);

        bool mInitialized{false};
        GLuint mProgram{0};
        GLuint mVAO{0};
        GLuint mVBO{0};
        
        int mViewWidth{0};
        int mViewHeight{0};
        float mIntensity{0.5f}; // Default intensity

        GLint mTextureLocation{-1};
        GLint mIntensityLocation{-1};
        GLint mFaceDataLocation{-1}; // Use uniform array for simplicity for now, or multiple uniforms
        
        // Max faces to support in shader
        static const int MAX_FACES = 5;
    };

} // face

#endif //FACEDEMO_FACELIFTRENDER_HPP
