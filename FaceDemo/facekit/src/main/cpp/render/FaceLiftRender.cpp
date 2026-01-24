//
// Created by wilbert on 2026/1/24.
//

#include "FaceLiftRender.hpp"
#include "common/Log.hpp"
#include <string>

namespace face {

    // Simple vertex shader: pass through texture coordinates
    static const char* VERTEX_SHADER_LIFT = R"(#version 300 es
    layout(location = 0) in vec4 aPosition;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 vTexCoord;
    void main() {
        gl_Position = aPosition;
        vTexCoord = aTexCoord;
    }
    )";

    // Fragment shader with face lift logic
    // We use a simple warping algorithm based on keypoints
    // For PFLD 98 points:
    // Left Face Contour: 0-32 (approx)
    // Chin: 16
    // We want to pull the contour points towards the center of the face
    
    // Note: This is a simplified implementation. A production-grade implementation
    // would typically use a grid mesh warping or a more complex fragment shader 
    // with optimized distance calculations.
    static const char* FRAGMENT_SHADER_LIFT = R"(#version 300 es
    precision mediump float;
    in vec2 vTexCoord;
    out vec4 fragColor;
    
    uniform sampler2D sTexture;
    uniform float uIntensity; // 0.0 to 1.0
    
    // Uniforms for face data
    // We support up to 5 faces. 
    // For each face, we need:
    // - Center of the face (or specific anchor points)
    // - Radius/Region of influence
    // - Target shift vector
    
    // To keep it simple for this demo, let's just implement a simple warp
    // that pushes pixels away from a "thinning" center.
    // Or rather, we want to "pull" the cheeks inwards.
    
    struct FaceInfo {
        int valid;
        vec2 leftCheek;  // Point to pull
        vec2 rightCheek; // Point to pull
        vec2 chin;       // Anchor
        vec2 center;     // Pull target (approx nose/center)
        float radius;    // Influence radius
    };
    
    uniform FaceInfo uFaces[5];

    // Warp function: moves current coordinate 'p' towards 'target' if within 'radius' of 'origin'
    vec2 warp(vec2 p, vec2 origin, vec2 target, float radius, float strength) {
        vec2 delta = p - origin;
        float dist = length(delta);
        float alpha = smoothstep(radius, 0.0, dist);
        if (alpha > 0.0) {
            vec2 shift = (target - origin) * alpha * strength * 0.5; 
            // In fragment shader, we are looking for "which source pixel corresponds to this coordinate"
            // So if we want to "shrink" the face (pull pixels IN), we actually need to look OUTWARDS.
            // So we subtract the shift.
            return p - shift;
        }
        return p;
    }

    void main() {
        vec2 coord = vTexCoord;
        
        // Apply warp for each face
        for (int i = 0; i < 5; i++) {
            if (uFaces[i].valid > 0) {
                // Left Cheek thinning
                // We define 'origin' as the cheek contour point
                // We define 'target' as the face center
                // We want pixels at 'origin' to come from 'target' direction? No.
                // We want the cheek to move IN. So the pixel at "Inner Cheek" should grab color from "Outer Cheek".
                // So coord should move OUT towards the contour.
                
                // Let's model it as:
                // We have a point C (Cheek Contour). We want it to move to C' (Inner).
                // So at position C', we want to sample C.
                // So shift vector is D = C - C'.
                // If we are at p = C', we want sample p + D.
                
                vec2 center = uFaces[i].center;
                vec2 leftCheek = uFaces[i].leftCheek;
                vec2 rightCheek = uFaces[i].rightCheek;
                float r = uFaces[i].radius;
                
                // Calculate direction vectors from cheek to center
                // This is the direction we want the face to move
                vec2 leftDir = normalize(center - leftCheek);
                vec2 rightDir = normalize(center - rightCheek);
                
                // Simple liquify effect:
                // At any point p, if it is close to the cheek region, offset it opposite to the movement direction
                // Movement: Cheek -> Center. 
                // Inverse Map: We look "Backwards" relative to movement.
                // So we look towards the Cheek (Away from Center).
                
                // Left side warp
                // Area of effect centered around the "Target" cheek position (slightly inwards from contour)
                vec2 leftTarget = leftCheek + leftDir * r * 0.2; 
                float dL = distance(coord, leftTarget);
                float alphaL = smoothstep(r, 0.0, dL);
                coord -= leftDir * alphaL * uIntensity * 0.05; // 0.05 is max displacement factor
                
                // Right side warp
                vec2 rightTarget = rightCheek + rightDir * r * 0.2;
                float dR = distance(coord, rightTarget);
                float alphaR = smoothstep(r, 0.0, dR);
                coord -= rightDir * alphaR * uIntensity * 0.05;
            }
        }
        
        fragColor = texture(sTexture, coord);
    }
    )";

    FaceLiftRender::FaceLiftRender() {}

    FaceLiftRender::~FaceLiftRender() {
        onDestroy();
    }

    void FaceLiftRender::init() {
        if (mInitialized) return;

        mProgram = createProgram(VERTEX_SHADER_LIFT, FRAGMENT_SHADER_LIFT);
        if (mProgram) {
            mTextureLocation = glGetUniformLocation(mProgram, "sTexture");
            mIntensityLocation = glGetUniformLocation(mProgram, "uIntensity");
            
            // Initialize buffer for full screen quad
            // x, y, u, v
            float vertices[] = {
                -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
                 1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
                -1.0f,  1.0f, 0.0f, 1.0f, // Top-left
                 1.0f,  1.0f, 1.0f, 1.0f  // Top-right
            };
            
            glGenVertexArrays(1, &mVAO);
            glGenBuffers(1, &mVBO);
            
            glBindVertexArray(mVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
            
            glBindVertexArray(0);
            
            mInitialized = true;
        }
    }

    void FaceLiftRender::setViewSize(int width, int height) {
        mViewWidth = width;
        mViewHeight = height;
    }

    void FaceLiftRender::setIntensity(float intensity) {
        mIntensity = intensity;
    }

    void FaceLiftRender::draw(GLuint textureId, const std::vector<BBox>& bboxes, int imageWidth, int imageHeight) {
        if (!mInitialized || mViewWidth == 0 || mViewHeight == 0) return;

        glUseProgram(mProgram);
        glViewport(0, 0, mViewWidth, mViewHeight);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(mTextureLocation, 0);
        glUniform1f(mIntensityLocation, mIntensity);

        // Upload face data
        int count = 0;
        for (const auto& bbox : bboxes) {
            if (count >= MAX_FACES) break;
            
            if (bbox.keypoints.size() == 98) {
                // PFLD 98 points
                // 0-32 is jawline
                // 16 is chin
                // 4 is roughly left cheek angle (on screen right if mirrored, but let's assume image coords)
                // 28 is roughly right cheek angle
                // Nose tip is 51-54, let's use 54 as center anchor
                
                // We normalize coordinates to [0, 1] for shader
                auto norm = [&](float x, float y) -> std::pair<float, float> {
                    return {x / imageWidth, y / imageHeight}; // UV coordinates: Top-left 0,0
                };
                
                // Left Cheek (approx index 3-5)
                auto pLeft = norm(bbox.keypoints[4].x, bbox.keypoints[4].y);
                // Right Cheek (approx index 27-29)
                auto pRight = norm(bbox.keypoints[28].x, bbox.keypoints[28].y);
                // Center (Nose tip 54)
                auto pCenter = norm(bbox.keypoints[54].x, bbox.keypoints[54].y);
                // Chin (16)
                auto pChin = norm(bbox.keypoints[16].x, bbox.keypoints[16].y);
                
                // Calculate rough radius based on face size
                float faceWidth = (bbox.x2 - bbox.x1) / imageWidth;
                float radius = faceWidth * 0.35f;

                std::string base = "uFaces[" + std::to_string(count) + "]";
                glUniform1i(glGetUniformLocation(mProgram, (base + ".valid").c_str()), 1);
                glUniform2f(glGetUniformLocation(mProgram, (base + ".leftCheek").c_str()), pLeft.first, pLeft.second);
                glUniform2f(glGetUniformLocation(mProgram, (base + ".rightCheek").c_str()), pRight.first, pRight.second);
                glUniform2f(glGetUniformLocation(mProgram, (base + ".chin").c_str()), pChin.first, pChin.second);
                glUniform2f(glGetUniformLocation(mProgram, (base + ".center").c_str()), pCenter.first, pCenter.second);
                glUniform1f(glGetUniformLocation(mProgram, (base + ".radius").c_str()), radius);
                
                count++;
            }
        }
        
        // Mark remaining faces as invalid
        for (int i = count; i < MAX_FACES; i++) {
            std::string base = "uFaces[" + std::to_string(i) + "]";
            glUniform1i(glGetUniformLocation(mProgram, (base + ".valid").c_str()), 0);
        }

        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void FaceLiftRender::onDestroy() {
        if (mInitialized) {
            if (mVBO) glDeleteBuffers(1, &mVBO);
            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mProgram) glDeleteProgram(mProgram);
            mInitialized = false;
        }
    }
    
    // Helper to compile shaders (duplicate from BBoxRender, should refactor ideally)
    GLuint FaceLiftRender::loadShader(GLenum type, const char* shaderCode) {
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
                LOGE("FaceLift Error compiling shader:\n%s\n", infoLog);
                free(infoLog);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint FaceLiftRender::createProgram(const char* vertexSource, const char* fragmentSource) {
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
