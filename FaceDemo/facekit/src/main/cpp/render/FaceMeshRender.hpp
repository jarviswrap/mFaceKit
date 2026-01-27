//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_FACEMESHRENDER_HPP
#define FACEDEMO_FACEMESHRENDER_HPP

#include <GLES3/gl3.h>
#include <vector>
#include "common/FaceBox.hpp"
#include "render/utils/Delaunay.hpp"

namespace face {

    /**
     * @brief 基于 Delaunay 三角剖分的瘦脸渲染器 (Mesh-based Face Lift with Delaunay)
     * 
     * 动态生成网格：
     * 1. 收集人脸关键点 + 屏幕边界点。
     * 2. 每一帧（或当关键点变化时）执行 Delaunay 三角剖分。
     * 3. 在 Vertex Shader 中对关键点进行位移，实现瘦脸。
     */
    class FaceMeshRender {
    public:
        FaceMeshRender();
        ~FaceMeshRender();

        void init();
        void setViewSize(int width, int height);
        void setIntensity(float intensity);
        void draw(GLuint textureId, const std::vector<BBox>& bboxes, int imageWidth, int imageHeight);
        void onDestroy();

    private:
        GLuint loadShader(GLenum type, const char* shaderCode);
        GLuint createProgram(const char* vertexSource, const char* fragmentSource);

        bool mInitialized{false};
        GLuint mProgram{0};
        GLuint mVAO{0};
        GLuint mVBO{0};
        GLuint mIBO{0}; 
        
        int mIndexCount{0};

        int mViewWidth{0};
        int mViewHeight{0};
        float mIntensity{0.05f};

        // Uniform Locations
        GLint mTextureLocation{-1};
        GLint mIntensityLocation{-1};
        
        static const int MAX_FACES = 5;
    };

} // face

#endif //FACEDEMO_FACEMESHRENDER_HPP
