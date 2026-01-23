//
// Created by wilbert on 2026/1/23.
//

#include "BBoxRender.hpp"
#include "common/Log.hpp"

namespace face {

    static const char* VERTEX_SHADER_BBOX = R"(#version 300 es
    layout(location = 0) in vec2 aPosition;
    uniform vec2 uScale;
    void main() {
        // Convert from image coordinates to NDC [-1, 1]
        // Image coordinates: (0,0) top-left, (w,h) bottom-right
        // NDC: (-1,1) top-left, (1,-1) bottom-right
        
        // x_ndc = (x / w) * 2 - 1
        // y_ndc = 1 - (y / h) * 2
        
        // We use uScale to pass 1/w and 1/h for aspect ratio correction if needed
        // But simpler: just pass raw coordinates and transform in shader?
        // Or transform in CPU. Let's transform in CPU to handle aspect ratio properly
        // wait, if we use the same coordinate system as image, we can just map it.
        
        // But wait, the previous render might have scaled/cropped the image.
        // We need to match that transformation.
        // For now, let's assume FitXY (stretched) to keep it simple, 
        // or we need to pass the same transform matrix as PixelRender.
        
        // Re-reading requirements: 
        // "BBox的(x1,y1,x2,y2)用来描述一个在(0,0,mResolution.getWidth(), mResolution.getHeight())区域内的矩形框"
        
        gl_Position = vec4(aPosition, 0.0, 1.0);
        gl_PointSize = 20.0;
    }
    )";

    static const char* FRAGMENT_SHADER_BBOX = R"(#version 300 es
    precision mediump float;
    uniform vec4 uColor;
    out vec4 fragColor;
    void main() {
        fragColor = uColor;
    }
    )";

    BBoxRender::BBoxRender() {}

    BBoxRender::~BBoxRender() {
        onDestroy();
    }

    void BBoxRender::init() {
        if (mInitialized) return;

        mProgram = createProgram(VERTEX_SHADER_BBOX, FRAGMENT_SHADER_BBOX);
        if (mProgram) {
            mColorLocation = glGetUniformLocation(mProgram, "uColor");
            // mScaleLocation = glGetUniformLocation(mProgram, "uScale");
            
            glGenVertexArrays(1, &mVAO);
            glGenBuffers(1, &mVBO);
            
            glBindVertexArray(mVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            
            // Just allocate some initial space, we'll update it every frame
            // 2 floats per vertex
            glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
            
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            
            glBindVertexArray(0);
            
            mInitialized = true;
        }
    }

    void BBoxRender::draw(const std::vector<BBox>& bboxes, int viewWidth, int viewHeight, int imageWidth, int imageHeight, float scaleX, float scaleY) {
        if (!mInitialized || bboxes.empty() || viewWidth == 0 || viewHeight == 0 || imageWidth == 0 || imageHeight == 0) {
            return;
        }

        glUseProgram(mProgram);
        
        // Prepare vertex data
        // Each bbox has 4 lines (8 vertices) + 5 landmarks (5 points)
        // We'll draw lines for box and points for landmarks
        
        std::vector<float> lineVertices;
        std::vector<float> pointVertices;

        for (const auto& bbox : bboxes) {
            // Box
            float x1 = bbox.x1;
            float y1 = bbox.y1;
            float x2 = bbox.x2;
            float y2 = bbox.y2;
            
            // Transform to NDC
            auto toNDC = [&](float x, float y) -> std::pair<float, float> {
                 float x_norm = x / imageWidth;
                 float y_norm = y / imageHeight;
                 float ndc_x = scaleX * (2.0f * x_norm - 1.0f);
                 float ndc_y = scaleY * (1.0f - 2.0f * y_norm); // Flip Y
                 return {ndc_x, ndc_y};
            };
            
            auto p1 = toNDC(x1, y1); // Top-left
            auto p2 = toNDC(x2, y1); // Top-right
            auto p3 = toNDC(x2, y2); // Bottom-right
            auto p4 = toNDC(x1, y2); // Bottom-left
            
            // Lines: p1->p2, p2->p3, p3->p4, p4->p1
            lineVertices.insert(lineVertices.end(), {p1.first, p1.second, p2.first, p2.second});
            lineVertices.insert(lineVertices.end(), {p2.first, p2.second, p3.first, p3.second});
            lineVertices.insert(lineVertices.end(), {p3.first, p3.second, p4.first, p4.second});
            lineVertices.insert(lineVertices.end(), {p4.first, p4.second, p1.first, p1.second});
            
            // Landmarks
            for (int i = 0; i < 5; i++) {
                auto p = toNDC(bbox.landmarks[i].x, bbox.landmarks[i].y);
                pointVertices.insert(pointVertices.end(), {p.first, p.second});
            }
        }
        
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        
        // Draw Lines (Green)
        if (!lineVertices.empty()) {
            glLineWidth(5.0f);
            glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_DYNAMIC_DRAW);
            glUniform4f(mColorLocation, 0.0f, 1.0f, 0.0f, 1.0f); // Green
            glDrawArrays(GL_LINES, 0, lineVertices.size() / 2);
        }
        
        // Draw Points (Red)
        if (!pointVertices.empty()) {
            glBufferData(GL_ARRAY_BUFFER, pointVertices.size() * sizeof(float), pointVertices.data(), GL_DYNAMIC_DRAW);
            glUniform4f(mColorLocation, 1.0f, 0.0f, 0.0f, 1.0f); // Red
            glDrawArrays(GL_POINTS, 0, pointVertices.size() / 2);
        }
        
        glBindVertexArray(0);
    }
    
    // ... helper functions ...
    
    GLuint BBoxRender::loadShader(GLenum type, const char* shaderCode) {
        GLuint shader = glCreateShader(type);
        if (shader == 0) return 0;
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char* infoLog = (char*)malloc(sizeof(char) * infoLen);
                glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                LOGE("Error compiling shader:\n%s\n", infoLog);
                free(infoLog);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint BBoxRender::createProgram(const char* vertexSource, const char* fragmentSource) {
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
    
    void BBoxRender::onDestroy() {
        if (mInitialized) {
            if (mVBO) glDeleteBuffers(1, &mVBO);
            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mProgram) glDeleteProgram(mProgram);
            mInitialized = false;
        }
    }

} // face
