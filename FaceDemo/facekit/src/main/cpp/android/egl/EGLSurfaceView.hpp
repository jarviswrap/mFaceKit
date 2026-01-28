//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLSURFACEVIEW_HPP
#define FACEDEMO_EGLSURFACEVIEW_HPP
#include "io/Consumer.hpp"
#include "common/PixelData.hpp"
#include "common/Size.hpp"
#include <jni.h>

namespace face {
    class EGLEnvironment;
    class PixelRender;
    class EGLSurfaceView: public std::enable_shared_from_this<EGLSurfaceView>{
    public:
        EGLSurfaceView(JNIEnv *env, jobject glsurfaceview);
        ~EGLSurfaceView();

        void initEnvironment(std::shared_ptr<EGLEnvironment> environment);

        void setSurfaceListener(DataListener<uint32_t, uint32_t> listener) { mSurfaceListener = listener; }
        void setDrawListener(DataListener<> listener) { mDrawListener = listener; }
        void setSurfaceDestroyListener(DataListener<> listener) { mSurfaceDestroyListener = listener; }

        Error requestDraw(); // requestDraw和destroy来自消费者线程
        Error destroy();

        void onDraw();

        void getSurfaceSize(uint32_t& width, uint32_t& height);

    protected:
        void onSurfaceChanged(uint32_t width, uint32_t height);
        void onSurfaceDestroy();

    private:
        DataListener<uint32_t, uint32_t> mSurfaceListener{nullptr};
        DataListener<> mDrawListener{nullptr};
        DataListener<> mSurfaceDestroyListener{nullptr};
        Size<uint32_t> mSurfaceSize{0, 0};
        std::atomic<uint32_t> mRequestIndex{0};
        std::atomic<uint32_t> mDrawIndex{0};
        jobject mSurfaceView{nullptr};
        jclass mSurfaceViewClass{nullptr};
        jmethodID mRequestRenderMethodID{nullptr};
        std::shared_ptr<EGLEnvironment> mEnvironment{nullptr};
        std::shared_ptr<PixelRender> mRender{nullptr};
        std::shared_ptr<PixelData> mCurrentData;
        float mFaceLiftIntensity{0};
    };

} // face

#endif //FACEDEMO_EGLSURFACEVIEW_HPP
