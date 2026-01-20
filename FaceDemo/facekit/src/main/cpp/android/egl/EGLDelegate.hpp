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

namespace face {

    class EGLDelegate {
    public:
        static EGLDelegate& getInstance();

        int64_t createEGLEnvironment();
        std::shared_ptr<EGLEnvironment> getEGLEnvironment(int64_t environment_ptr);
        void removeEGLEnvironment(int64_t environment_ptr);

        int64_t getSharedContext();
    private:
        EGLDelegate() = default;
        ~EGLDelegate() = default;
        EGLDelegate(const EGLDelegate&) = delete;
        EGLDelegate& operator=(const EGLDelegate&) = delete;

        std::unordered_map<int64_t, std::shared_ptr<EGLEnvironment>> mEnvironments;
        std::mutex mMutex;
    };

} // face

#endif //FACEDEMO_EGLDELEGATE_HPP
