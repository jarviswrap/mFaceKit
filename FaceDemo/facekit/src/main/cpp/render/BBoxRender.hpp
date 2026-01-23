//
// Created by wilbert on 2026/1/23.
//

#ifndef FACEDEMO_BBOXRENDER_HPP
#define FACEDEMO_BBOXRENDER_HPP

#include <GLES3/gl3.h>
#include <vector>
#include "common/FaceBox.hpp"
#include "common/Size.hpp"

namespace face {

    class BBoxRender {
    public:
        BBoxRender();
        ~BBoxRender();

        void init();
        void draw(const std::vector<BBox>& bboxes, int viewWidth, int viewHeight, int imageWidth, int imageHeight, float scaleX, float scaleY);
        void onDestroy();

    private:
        GLuint loadShader(GLenum type, const char* shaderCode);
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);

        bool mInitialized{false};
        GLuint mProgram{0};
        GLuint mVAO{0};
        GLuint mVBO{0};
        
        GLint mColorLocation{-1};
        GLint mScaleLocation{-1};
    };

} // face

#endif //FACEDEMO_BBOXRENDER_HPP
