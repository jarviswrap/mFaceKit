//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLSURFACEVIEW_HPP
#define FACEDEMO_EGLSURFACEVIEW_HPP
#include "io/Consumer.hpp"
#include "common/PixelData.hpp"
#include <jni.h>

namespace face {

    class EGLSurfaceView: public Consumer<PixelData> {
    public:
        EGLSurfaceView(JNIEnv *env, jobject glsurfaceview);
        ~EGLSurfaceView() override;

        Error onRequestConsume(uint32_t requestId) override;
        Error onConsumeData(PixelData data) override;
    };

} // face

#endif //FACEDEMO_EGLSURFACEVIEW_HPP
