//
// Created by wilbert on 2026/1/24.
//

#include "DelaunayDebugRender.hpp"
#include "common/Log.hpp"
#include <string>

namespace face {

    // 简单的线框着色器
    static const char* DEBUG_VERTEX_SHADER = R"(#version 300 es
    layout(location = 0) in vec2 aPosition;
    void main() {
        gl_Position = vec4(aPosition, 0.0, 1.0);
    }
    )";

    static const char* DEBUG_FRAGMENT_SHADER = R"(#version 300 es
    precision mediump float;
    out vec4 fragColor;
    uniform vec4 uColor;
    void main() {
        fragColor = uColor;
    }
    )";

    DelaunayDebugRender::DelaunayDebugRender() {}

    DelaunayDebugRender::~DelaunayDebugRender() {
        onDestroy();
    }

    void DelaunayDebugRender::init() {
        if (mInitialized) return;

        mProgram = createProgram(DEBUG_VERTEX_SHADER, DEBUG_FRAGMENT_SHADER);
        if (mProgram) {
            mColorLocation = glGetUniformLocation(mProgram, "uColor");
            
            glGenVertexArrays(1, &mVAO);
            glGenBuffers(1, &mVBO);
            glGenBuffers(1, &mIBO);

            mInitialized = true;
        }
    }

    void DelaunayDebugRender::setViewSize(int width, int height) {
        mViewWidth = width;
        mViewHeight = height;
    }

    void DelaunayDebugRender::draw(const std::vector<BBox>& bboxes, int imageWidth, int imageHeight) {
        if (!mInitialized || mViewWidth == 0 || mViewHeight == 0) return;

        // 1. Collect Points (Same logic as FaceMeshRender)
        std::vector<DPoint> points;
        points.reserve(120);

        points.push_back({0.0f, 0.0f, -1});
        points.push_back({1.0f, 0.0f, -2});
        points.push_back({0.0f, 1.0f, -3});
        points.push_back({1.0f, 1.0f, -4});
        
        points.push_back({0.5f, 0.0f, -5});
        points.push_back({0.5f, 1.0f, -6});
        points.push_back({0.0f, 0.5f, -7});
        points.push_back({1.0f, 0.5f, -8});

        bool hasFace = false;
        for (const auto& bbox : bboxes) {
            if (bbox.keypoints.size() == 98) {
                 for (int i = 0; i < 98; i++) {
                     float u = bbox.keypoints[i].x / imageWidth;
                     float v = bbox.keypoints[i].y / imageHeight;
                     points.push_back({u, v, i});
                 }
                 hasFace = true;
                 break;
            }
        }
        
        if (points.size() < 10) return;

        // 2. Run Delaunay
        std::vector<int> indices = Delaunay::triangulate(points);
        
        // Convert triangle indices to line indices (Wireframe)
        // Each triangle (a, b, c) becomes 3 lines: (a,b), (b,c), (c,a)
        std::vector<int> lineIndices;
        lineIndices.reserve(indices.size() * 2);
        for (size_t i = 0; i < indices.size(); i += 3) {
            int a = indices[i];
            int b = indices[i+1];
            int c = indices[i+2];
            
            lineIndices.push_back(a); lineIndices.push_back(b);
            lineIndices.push_back(b); lineIndices.push_back(c);
            lineIndices.push_back(c); lineIndices.push_back(a);
        }
        mIndexCount = lineIndices.size();

        // 3. Update Buffers
        std::vector<float> vertices;
        vertices.reserve(points.size() * 2);

        for (const auto& p : points) {
            // Map [0, 1] to NDC [-1, 1]
            float px = p.x * 2.0f - 1.0f;
            float py = (1.0f - p.y) * 2.0f - 1.0f; // Invert Y
            vertices.push_back(px);
            vertices.push_back(py);
        }

        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(int), lineIndices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 4. Draw
        glUseProgram(mProgram);
        
        // Draw Green Lines
        glLineWidth(2.0f); // Note: lineWidth > 1.0 might not be supported on all ES 3.0 implementations
        glUniform4f(mColorLocation, 0.0f, 1.0f, 0.0f, 1.0f);
        
        glDrawElements(GL_LINES, mIndexCount, GL_UNSIGNED_INT, 0);
        
        // Draw Points (Red)
        glUniform4f(mColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 2);
        
        glBindVertexArray(0);
    }

    void DelaunayDebugRender::onDestroy() {
        if (mInitialized) {
            if (mVBO) glDeleteBuffers(1, &mVBO);
            if (mIBO) glDeleteBuffers(1, &mIBO);
            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mProgram) glDeleteProgram(mProgram);
            mInitialized = false;
        }
    }

    GLuint DelaunayDebugRender::loadShader(GLenum type, const char* shaderCode) {
        GLuint shader = glCreateShader(type);
        if (shader == 0) return 0;
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint DelaunayDebugRender::createProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertexShader) return 0;
        GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (!fragmentShader) return 0;
        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            GLint linkStatus;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (!linkStatus) {
                glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

} // face
