//
// Created by wilbert on 2026/1/20.
//
#include <jni.h>
#include <android/native_window_jni.h>

#include "EGLDelegate.hpp"
#include "android/AndroidUtils.hpp"

using namespace face;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_createEGLEnvironment(JNIEnv *env, jobject thiz) {
    return EGLDelegate::getInstance().createEGLEnvironment();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeRelease(JNIEnv *env, jobject thiz, jlong environment_ptr) {
    EGLDelegate::getInstance().removeEGLEnvironment(environment_ptr);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLCreateContext(JNIEnv *env,
                                                                  jobject thiz,
                                                                  jlong environment_ptr,
                                                                  jlong shared_context_ptr,
                                                                  jint scene) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->createEGLContext(shared_context_ptr, static_cast<uint8_t>(scene));
    }
    return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLDestroyContext(JNIEnv *env,
                                                                   jobject thiz,
                                                                   jlong environment_ptr) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->destroyEGLContext();
    }
    return false;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLCreateWindowSurface(JNIEnv *env,
                                                                        jobject thiz,
                                                                        jlong environment_ptr,
                                                                        jobject surface) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        ANativeWindow *window = ANativeWindow_fromSurface(env, reinterpret_cast<jobject>(surface));
        return eglEnv->createWindowEGLSurface(window);
    }
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLCreatePBufferSurface(JNIEnv *env,
                                                                         jobject thiz,
                                                                         jlong environment_ptr,
                                                                         jint width,
                                                                         jint height) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->createPBufferEGLSurface(width, height);
    }
    return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLDestroySurface(JNIEnv *env,
                                                                   jobject thiz,
                                                                   jlong environment_ptr) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->destroyEGLSurface();
    }
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_nativeEGLMakeCurrent(JNIEnv *env,
                                                                jobject thiz,
                                                                jlong environment_ptr) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->makeEGLCurrent();
    }
    return false;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_getEGLSurfaceWidth(JNIEnv *env,
                                                              jobject thiz,
                                                              jlong environment_ptr) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->getEGLSurfaceWidth();
    }
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jarvis_facekit_egl_EGLEnvironment_getEGLSurfaceHeight(JNIEnv *env,
                                                               jobject thiz,
                                                               jlong environment_ptr) {
    auto eglEnv = EGLDelegate::getInstance().getEGLEnvironment(environment_ptr);
    if (eglEnv) {
        return eglEnv->getEGLSurfaceHeight();
    }
    return 0;
}
