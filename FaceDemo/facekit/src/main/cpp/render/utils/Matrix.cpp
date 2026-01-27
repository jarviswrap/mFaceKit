//
// Created by TraeAI on 2026/1/27.
//

#include "Matrix.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace face {

    Matrix::Matrix() {
        reset();
    }

    void Matrix::reset() {
        memset(mMat, 0, sizeof(mMat));
        mMat[0] = 1.0f;
        mMat[5] = 1.0f;
        mMat[10] = 1.0f;
        mMat[15] = 1.0f;
    }

    const float* Matrix::peek() const {
        return mMat;
    }

    void Matrix::multiply(const float* rhs) {
        float temp[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                float sum = 0.0f;
                for (int k = 0; k < 4; k++) {
                    // mMat is LHS, rhs is RHS
                    // Result(row=j, col=i) = Sum(LHS(row=j, k) * RHS(row=k, col=i))
                    // Since we store column-major:
                    // Element at row r, col c is index (c * 4 + r)
                    // LHS(j, k) -> mMat[k * 4 + j]
                    // RHS(k, i) -> rhs[i * 4 + k]
                    sum += mMat[k * 4 + j] * rhs[i * 4 + k];
                }
                temp[i * 4 + j] = sum;
            }
        }
        memcpy(mMat, temp, sizeof(mMat));
    }

    void Matrix::postRotate(float angle, float px, float py) {
        if (angle == 0.0f) return;

        float rad = angle * (float)M_PI / 180.0f;
        float c = cosf(rad);
        float s = sinf(rad);

        // Rotation matrix around Z axis
        // | c -s  0  0 |
        // | s  c  0  0 |
        // | 0  0  1  0 |
        // | 0  0  0  1 |
        
        // If px, py != 0, we do Translate(px, py) * Rotate * Translate(-px, -py)
        // But here we implement post-multiplication: Current = Rotate * Current
        // The user asked for "postRotate" similar to MNN/Android which usually means
        // the new transform is applied AFTER the existing one (M' = T * M) or BEFORE?
        // Android Matrix postRotate: M' = T * M (T is rotation)
        // OpenGL: usually we multiply projection * view * model.
        // Let's implement M' = T * M.
        
        float temp[16];
        memset(temp, 0, sizeof(temp));
        temp[10] = 1.0f;
        temp[15] = 1.0f;

        // Translate(px, py) * Rotate * Translate(-px, -py)
        // This is equivalent to rotating around (px, py).
        
        if (px == 0.0f && py == 0.0f) {
             temp[0] = c; temp[4] = -s;
             temp[1] = s; temp[5] = c;
             multiply(temp);
        } else {
             postTranslate(-px, -py);
             
             memset(temp, 0, sizeof(temp));
             temp[10] = 1.0f; temp[15] = 1.0f;
             temp[0] = c; temp[4] = -s;
             temp[1] = s; temp[5] = c;
             multiply(temp);
             
             postTranslate(px, py);
        }
    }

    void Matrix::postScale(float sx, float sy, float px, float py) {
        float temp[16];
        memset(temp, 0, sizeof(temp));
        temp[0] = sx;
        temp[5] = sy;
        temp[10] = 1.0f;
        temp[15] = 1.0f;

        if (px == 0.0f && py == 0.0f) {
            multiply(temp);
        } else {
            postTranslate(-px, -py);
            multiply(temp);
            postTranslate(px, py);
        }
    }

    void Matrix::postTranslate(float tx, float ty) {
        float temp[16];
        memset(temp, 0, sizeof(temp));
        temp[0] = 1.0f;
        temp[5] = 1.0f;
        temp[10] = 1.0f;
        temp[15] = 1.0f;
        
        // Translation in column 3 (index 12, 13, 14)
        temp[12] = tx;
        temp[13] = ty;
        
        multiply(temp);
    }

} // namespace face
