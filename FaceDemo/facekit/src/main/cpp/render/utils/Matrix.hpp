//
// Created by TraeAI on 2026/1/27.
//

#ifndef FACEDEMO_MATRIX_HPP
#define FACEDEMO_MATRIX_HPP

#include <cstring>
#include <cmath>

namespace face {

    class Matrix {
    public:
        Matrix();
        ~Matrix() = default;

        // Resets the matrix to identity
        void reset();

        // Applies a rotation around the Z axis
        // angle: in degrees
        // px, py: pivot point
        void postRotate(float angle, float px, float py);

        // Applies a scaling
        // sx, sy: scale factors
        // px, py: pivot point
        void postScale(float sx, float sy, float px, float py);

        // Applies a translation
        void postTranslate(float tx, float ty);

        // Returns the 16-element array (column-major) for OpenGL
        const float* peek() const;

    private:
        // Column-major 4x4 matrix
        float mMat[16];

        void multiply(const float* rhs);
    };

} // namespace face

#endif //FACEDEMO_MATRIX_HPP
