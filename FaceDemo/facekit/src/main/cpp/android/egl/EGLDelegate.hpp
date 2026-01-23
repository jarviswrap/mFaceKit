//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLDELEGATE_HPP
#define FACEDEMO_EGLDELEGATE_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include <jni.h>
#include <android/native_window.h>
#include "common/Common.hpp"
namespace face {
    class ImagePreviewer;
    class EGLEnvironment;
    class EGLSurfaceView;
    class EGLDelegate {
    public:
        static EGLDelegate& getInstance();
        EGLDelegate(const EGLDelegate&) = delete;
        EGLDelegate& operator=(const EGLDelegate&) = delete;

        int64_t createEGLEnvironment();
        std::shared_ptr<EGLEnvironment> getEGLEnvironment(int64_t environment_ptr);
        void removeEGLEnvironment(int64_t environment_ptr);

        int64_t getSharedContext();

        int64_t createEGLSurfaceView(JNIEnv* env, jobject eglSurfaceView, int64_t eglEnvironmentPtr);
        std::shared_ptr<EGLSurfaceView> getShowView(int64_t surfaceview_ptr = 0);
        void removeEGLShowView(int64_t surfaceview_ptr);
        void setShowViewListener(DataListener<std::shared_ptr<EGLSurfaceView>> listener) { mShowViewListener = listener; }

    private:
        EGLDelegate() = default;
        ~EGLDelegate() = default;



        std::mutex mMutex;
        std::unordered_map<int64_t, std::shared_ptr<EGLEnvironment>> mEnvironments;

        std::unordered_map<int64_t, std::shared_ptr<EGLSurfaceView>> mSurfaceViews;
        std::shared_ptr<EGLSurfaceView> mLastShowView{nullptr};

        DataListener<std::shared_ptr<EGLSurfaceView>> mShowViewListener;
    };

} // face

#endif //FACEDEMO_EGLDELEGATE_HPP
