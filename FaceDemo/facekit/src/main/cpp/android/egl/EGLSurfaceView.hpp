//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLSURFACEVIEW_HPP
#define FACEDEMO_EGLSURFACEVIEW_HPP
#include "io/Consumer.hpp"
#include "common/PixelData.hpp"
#include <jni.h>

namespace face {
    class EGLEnvironment;
    class PixelRender;
    class EGLSurfaceView: public Consumer<PixelData> {
    public:
        EGLSurfaceView(JNIEnv *env, jobject glsurfaceview, std::shared_ptr<EGLEnvironment> environment);
        ~EGLSurfaceView() override;

        Error onRequestConsume(uint32_t requestId) override;
        void onDestroy() override; // onRequestConsume和onDestroy都来自生产者线程

        Error onConsumeData(const std::shared_ptr<PixelData>& data) override; //onConsumeData来自消费者线程
    private:
        jobject mSurfaceView{nullptr};
        jclass mSurfaceViewClass{nullptr};
        jmethodID mRequestRenderMethodID{nullptr};
        std::shared_ptr<EGLEnvironment> mEnvironment{nullptr};
        std::shared_ptr<PixelRender> mRender{nullptr};
    };

} // face

#endif //FACEDEMO_EGLSURFACEVIEW_HPP
