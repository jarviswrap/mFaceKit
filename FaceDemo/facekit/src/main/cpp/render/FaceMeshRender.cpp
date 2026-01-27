//
// Created by wilbert on 2026/1/24.
//

#include "FaceMeshRender.hpp"
#include "common/Log.hpp"
#include <string>
#include <cmath>

namespace face {

    // Vertex Shader: 执行网格变形逻辑
    // 相比 Fragment Shader 的“逆向采样”，Vertex Shader 做的是“正向位移”。
    // 即：将脸颊处的顶点向中心“挤压”。
    static const char* MESH_VERTEX_SHADER = R"(#version 300 es
    layout(location = 0) in vec4 aPosition; // NDC [-1, 1]
    layout(location = 1) in vec2 aTexCoord; // [0, 1]
    
    out vec2 vTexCoord;
    
    uniform float uIntensity;
    
    const int CHEEK_COUNT = 12;
    struct FaceInfo {
        int valid;
        int leftCount;
        int rightCount;
        vec2 leftCheeks[CHEEK_COUNT];
        vec2 rightCheeks[CHEEK_COUNT];
        vec2 center;
        float radius;
        float falloff;
    };
    uniform FaceInfo uFaces[5];

    void main() {
        vec2 pos = aPosition.xy;
        // Adjust Y direction if needed (OpenGL NDC Y is up, Image Y is usually down, 
        // but here we assume texture is mapped correctly so 0,0 is bottom-left or top-left depending on setup.
        // Let's assume input coordinates match aTexCoord system.)
        
        // Actually, let's use aTexCoord for distance calculation as it aligns with image features
        vec2 currentPos = aTexCoord; 
        
        vec2 offset = vec2(0.0);
        
        for(int i = 0; i < 5; i++) {
            if (uFaces[i].valid > 0) {
                vec2 center = uFaces[i].center;
                float r = uFaces[i].radius;
                float rOuter = r * uFaces[i].falloff;
                int leftCount = uFaces[i].leftCount;
                int rightCount = uFaces[i].rightCount;
                
                for (int j = 0; j < CHEEK_COUNT; j++) {
                    if (j >= leftCount) break;
                    vec2 cheek = uFaces[i].leftCheeks[j];
                    float d = distance(currentPos, cheek);
                    if (d < rOuter) {
                        float alpha = smoothstep(rOuter, r, d);
                        vec2 dir = normalize(center - cheek);
                        offset += dir * alpha * uIntensity * 0.05;
                    }
                }
                
                for (int j = 0; j < CHEEK_COUNT; j++) {
                    if (j >= rightCount) break;
                    vec2 cheek = uFaces[i].rightCheeks[j];
                    float d = distance(currentPos, cheek);
                    if (d < rOuter) {
                        float alpha = smoothstep(rOuter, r, d);
                        vec2 dir = normalize(center - cheek);
                        offset += dir * alpha * uIntensity * 0.05;
                    }
                }
            }
        }
        
        // Apply offset to Texture Coordinate? Or Position?
        // If we move Position: The vertex moves on screen. The texture attached to it moves with it.
        // If we want to "shrink" the face (make it smaller), we should pull vertices INWARDS.
        // This will bring the texture from the cheek area towards the center.
        // But wait, if we pull vertices IN, the "Cheek Texture" moves to "Center". 
        // The "Ear Texture" (outside) stretches to cover the gap.
        // This effectively makes the face LOOK smaller (occupied by less screen space).
        // Correct.
        
        // However, aPosition is in NDC [-1, 1].
        // offset is calculated in [0, 1] space.
        // Need to scale offset to NDC space (multiply by 2).
        
        // Invert Y for offset if necessary?
        // Assuming aTexCoord (0,0) is Top-Left and aPosition (-1, 1) is Top-Left.
        // If aTexCoord Y increases downwards, but aPosition Y increases upwards.
        // Then +Y in TexCoord is -Y in Position.
        
        // Let's assume standard mapping:
        // aPosition (-1, -1) -> aTexCoord (0, 0) [Bottom-Left]
        // aPosition (1, 1)   -> aTexCoord (1, 1) [Top-Right]
        // Then they are aligned.
        
        gl_Position = vec4(pos + offset * 2.0, 0.0, 1.0);
        vTexCoord = aTexCoord;
    }
    )";

    // Fragment Shader: 简单的纹理采样
    static const char* MESH_FRAGMENT_SHADER = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    uniform sampler2D sTexture;
    
    void main() {
        fragColor = texture(sTexture, vTexCoord);
    }
    )";

    FaceMeshRender::FaceMeshRender() {}

    FaceMeshRender::~FaceMeshRender() {
        onDestroy();
    }

    void FaceMeshRender::init() {
        if (mInitialized) return;

        mProgram = createProgram(MESH_VERTEX_SHADER, MESH_FRAGMENT_SHADER);
        if (mProgram) {
            mTextureLocation = glGetUniformLocation(mProgram, "sTexture");
            mIntensityLocation = glGetUniformLocation(mProgram, "uIntensity");
            
            // initGrid(mGridCols, mGridRows); // No longer using static grid
            
            glGenVertexArrays(1, &mVAO);
            glGenBuffers(1, &mVBO);
            glGenBuffers(1, &mIBO);

            mInitialized = true;
        }
    }

    void FaceMeshRender::setViewSize(int width, int height) {
        mViewWidth = width;
        mViewHeight = height;
    }

    void FaceMeshRender::setIntensity(float intensity) {
        mIntensity = intensity;
    }

    // Dynamic mesh generation using Delaunay
    void FaceMeshRender::draw(GLuint textureId, const std::vector<BBox>& bboxes, int imageWidth, int imageHeight) {
        if (!mInitialized || mViewWidth == 0 || mViewHeight == 0) return;

        glUseProgram(mProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(mTextureLocation, 0);
        glUniform1f(mIntensityLocation, mIntensity);

        // 1. Collect Points for Delaunay
        std::vector<DPoint> points;
        points.reserve(120); // 98 landmarks + boundaries

        // Add Image Corners (Normalized [0, 1])
        points.push_back({0.0f, 0.0f, -1});
        points.push_back({1.0f, 0.0f, -2});
        points.push_back({0.0f, 1.0f, -3});
        points.push_back({1.0f, 1.0f, -4});
        
        // Add Edge Midpoints (to reduce long triangles)
        points.push_back({0.5f, 0.0f, -5});
        points.push_back({0.5f, 1.0f, -6});
        points.push_back({0.0f, 0.5f, -7});
        points.push_back({1.0f, 0.5f, -8});

        int faceCount = 0;
        
        // Add Face Landmarks
        for (const auto& bbox : bboxes) {
            if (bbox.keypoints.size() == 98) {
                 for (int i = 0; i < 98; i++) {
                     float u = bbox.keypoints[i].x / imageWidth;
                     float v = bbox.keypoints[i].y / imageHeight;
                     // Invert Y for GL Texture Coords if needed (assuming 0=bottom)
                     // But let's keep [0,1] logical space same as image for now
                     // We will handle V inversion when uploading to GL buffer
                     points.push_back({u, v, i});
                 }
                 faceCount++;
                 break; // Only support 1 face for mesh generation simplicity in this demo
            }
        }
        
        if (points.size() < 10) return; // Not enough points

        // 2. Run Delaunay
        std::vector<int> indices = Delaunay::triangulate(points);
        mIndexCount = indices.size();

        // 3. Update Buffers
        std::vector<float> vertices;
        vertices.reserve(points.size() * 4); // x, y, u, v

        for (const auto& p : points) {
            // Position: Map [0, 1] to NDC [-1, 1]
            float px = p.x * 2.0f - 1.0f;
            float py = (1.0f - p.y) * 2.0f - 1.0f; // Invert Y for screen position (GL Y is up, Image Y is down)

            // TexCoord: [0, 1]
            float u = p.x;
            float v = 1.0f - p.y; // Invert Y for texture sampling

            vertices.push_back(px);
            vertices.push_back(py);
            vertices.push_back(u);
            vertices.push_back(v);
        }

        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_DRAW);

        // Attr 0: Position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attr 1: TexCoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Upload Uniforms for Deformation (Same logic as before)
        // ... (Upload FaceInfo)
        int count = 0;
        for (const auto& bbox : bboxes) {
            if (count >= MAX_FACES) break;
            if (bbox.keypoints.size() == 98) {
                auto normPoint = [&](int idx) -> std::pair<float, float> {
                    const auto& pt = bbox.keypoints[idx];
                    float x = pt.x / imageWidth;
                    float y = pt.y / imageHeight;
                    return {x, 1.0f - y};
                };
                auto meanPoint = [&](const std::vector<int>& indices) -> std::pair<float, float> {
                    float sx = 0.0f;
                    float sy = 0.0f;
                    int n = 0;
                    for (int idx : indices) {
                        if (idx >= 0 && idx < static_cast<int>(bbox.keypoints.size())) {
                            auto p = normPoint(idx);
                            sx += p.first;
                            sy += p.second;
                            n++;
                        }
                    }
                    if (n == 0) {
                        return normPoint(54);
                    }
                    return {sx / n, sy / n};
                };
                auto collectPoints = [&](const std::vector<int>& indices) -> std::vector<std::pair<float, float>> {
                    std::vector<std::pair<float, float>> pts;
                    pts.reserve(indices.size());
                    for (int idx : indices) {
                        if (idx >= 0 && idx < static_cast<int>(bbox.keypoints.size())) {
                            pts.push_back(normPoint(idx));
                        }
                    }
                    return pts;
                };
                std::vector<int> leftIndices{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
                std::vector<int> rightIndices{19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
                std::vector<int> centerIndices{57, 79, 90, 94, 85};
                auto leftPoints = collectPoints(leftIndices);
                auto rightPoints = collectPoints(rightIndices);
                auto pCenter = meanPoint(centerIndices);

                float faceWidth = (bbox.x2 - bbox.x1) / imageWidth;
                float radius = faceWidth * 0.35f;
                float falloff = 1.6f;

                std::string base = "uFaces";
                glUniform1i(glGetUniformLocation(mProgram, (base + ".valid").c_str()), 1);
                glUniform1i(glGetUniformLocation(mProgram, (base + ".leftCount").c_str()), static_cast<int>(leftPoints.size()));
                glUniform1i(glGetUniformLocation(mProgram, (base + ".rightCount").c_str()), static_cast<int>(rightPoints.size()));
                for (int j = 0; j < static_cast<int>(leftPoints.size()); j++) {
                    std::string lname = base + ".leftCheeks";
                    glUniform2f(glGetUniformLocation(mProgram, lname.c_str()), leftPoints[j].first, leftPoints[j].second);
                }
                for (int j = 0; j < static_cast<int>(rightPoints.size()); j++) {
                    std::string rname = base + ".rightCheeks";
                    glUniform2f(glGetUniformLocation(mProgram, rname.c_str()), rightPoints[j].first, rightPoints[j].second);
                }
                glUniform2f(glGetUniformLocation(mProgram, (base + ".center").c_str()), pCenter.first, pCenter.second);
                glUniform1f(glGetUniformLocation(mProgram, (base + ".radius").c_str()), radius);
                glUniform1f(glGetUniformLocation(mProgram, (base + ".falloff").c_str()), falloff);
                count++;
            }
        }
        for (int i = count; i < MAX_FACES; i++) {
            std::string base = "uFaces";
            glUniform1i(glGetUniformLocation(mProgram, (base + ".valid").c_str()), 0);
        }

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void FaceMeshRender::onDestroy() {
        if (mInitialized) {
            if (mVBO) glDeleteBuffers(1, &mVBO);
            if (mIBO) glDeleteBuffers(1, &mIBO);
            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mProgram) glDeleteProgram(mProgram);
            mInitialized = false;
        }
    }

    // Shader Helpers (Copied for simplicity, should be in utility)
    GLuint FaceMeshRender::loadShader(GLenum type, const char* shaderCode) {
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
                LOGE("MeshRender Error compiling shader:\n%s\n", infoLog);
                free(infoLog);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint FaceMeshRender::createProgram(const char* vertexSource, const char* fragmentSource) {
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
