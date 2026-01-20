//
// Created by wilbert on 2026/1/20.
//

#ifndef FACEDEMO_EGLDELEGATE_HPP
#define FACEDEMO_EGLDELEGATE_HPP

#include <unordered_map>
#include <memory>
#include <mutex>

#include <android/native_window.h>
#include "EGLEnvironment.hpp"

#include "EGLSurfaceView.hpp"

namespace face {

    class EGLDelegate {
    public:
        static EGLDelegate& getInstance();

        int64_t createEGLEnvironment();
        std::shared_ptr<EGLEnvironment> getEGLEnvironment(int64_t environment_ptr);
        void removeEGLEnvironment(int64_t environment_ptr);

        int64_t getSharedContext();

        int64_t createEGLSurfaceView(JNIEnv* env, jobject eglSurfaceView);
        void onSurfaceViewDraw(int64_t surfaceview_ptr);

    private:
        EGLDelegate() = default;
        ~EGLDelegate() = default;
        EGLDelegate(const EGLDelegate&) = delete;
        EGLDelegate& operator=(const EGLDelegate&) = delete;

        std::mutex mMutex;
        std::unordered_map<int64_t, std::shared_ptr<EGLEnvironment>> mEnvironments;

        std::unordered_map<int64_t, std::shared_ptr<EGLSurfaceView>> mSurfaceViews;
    };

} // face

#endif //FACEDEMO_EGLDELEGATE_HPP
