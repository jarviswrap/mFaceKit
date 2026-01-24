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

    /**
     * @brief 瘦脸特效渲染器 (Face Lift Render)
     * 
     * 基于 OpenGL ES 3.0 实现的瘦脸/液化滤镜。
     * 使用 Fragment Shader 对人脸区域（主要是左右脸颊）进行像素位移（Warping），
     * 从而达到“瘦脸”或“修容”的视觉效果。
     */
    class FaceLiftRender {
    public:
        FaceLiftRender();
        ~FaceLiftRender();

        /**
         * @brief 初始化 OpenGL 资源
         * 创建并编译 Shader Program，初始化 VAO/VBO 等。
         * 必须在 GL 线程中调用。
         */
        void init();

        /**
         * @brief 设置渲染视图尺寸
         * 通常对应 GLSurfaceView 或 FBO 的宽高。
         * @param width 视图宽度
         * @param height 视图高度
         */
        void setViewSize(int width, int height);

        /**
         * @brief 设置瘦脸强度
         * @param intensity 强度值，范围 [0.0, 1.0]。0.0 为无效果，1.0 为最大效果。
         */
        void setIntensity(float intensity); // 0.0 - 1.0

        /**
         * @brief 执行渲染绘制
         * 
         * 绑定输入纹理，上传人脸关键点数据到 Uniform，并绘制全屏四边形以应用滤镜。
         * 
         * @param textureId 输入纹理 ID (通常是包含人脸图像的 FBO 纹理)
         * @param bboxes 包含人脸关键点的人脸框列表
         * @param imageWidth 原始图像宽度（用于归一化关键点坐标）
         * @param imageHeight 原始图像高度
         */
        void draw(GLuint textureId, const std::vector<BBox>& bboxes, int imageWidth, int imageHeight);

        /**
         * @brief 销毁 OpenGL 资源
         * 释放 Shader, Program, VAO, VBO 等。
         */
        void onDestroy();

    private:
        /**
         * @brief 加载并编译 Shader
         */
        GLuint loadShader(GLenum type, const char* shaderCode);
        /**
         * @brief 创建 Shader Program
         */
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
