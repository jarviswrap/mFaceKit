//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_DELAUNAYDEBUGRENDER_HPP
#define FACEDEMO_DELAUNAYDEBUGRENDER_HPP

#include <GLES3/gl3.h>
#include <vector>
#include "common/FaceBox.hpp"
#include "render/utils/Delaunay.hpp"

namespace face {

    /**
     * @brief Delaunay 三角剖分调试渲染器
     * 
     * 用于可视化 FaceMeshRender 生成的三角网格线框。
     * 这有助于验证网格是否正确覆盖了人脸区域以及背景。
     * 
     * 渲染逻辑：
     * 1. 使用与 FaceMeshRender 相同的逻辑生成 Delaunay 三角形。
     * 2. 使用 GL_LINES 模式绘制三角形的边。
     * 3. 叠加在原始画面之上。
     */
    class DelaunayDebugRender {
    public:
        DelaunayDebugRender();
        ~DelaunayDebugRender();

        void init();
        void setViewSize(int width, int height);
        void draw(const std::vector<BBox>& bboxes, int imageWidth, int imageHeight);
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

        GLint mColorLocation{-1};
    };

} // face

#endif //FACEDEMO_DELAUNAYDEBUGRENDER_HPP
