#include <jni.h>
#include <string>
//#include "engine/FaceInference.hpp"
#include "android/AndroidUtils.hpp"
#include "common/Log.hpp"
#include "JNIEnvManager.hpp"
#include "android/example/ImagePreviewer.hpp"
#include "android/AndroidUtils.hpp"
#include "core/RetinaFace.hpp"

static std::shared_ptr<face::ImagePreviewer> sImagePreviewer = nullptr;

//std::shared_ptr<face::FaceInference> mInference;
extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGE("JNI_OnLoad");
    face::JNIEnvManager::getInstance().initJavaVM(vm);
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_jarvis_facekit_FaceKit_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_jarvis_facekit_FaceKit_setModelDir(
        JNIEnv* env,
        jobject /* this */, jstring imagePath) {
    if (!imagePath && sImagePreviewer) {
        sImagePreviewer.reset();
        return;
    }
    if (!sImagePreviewer) {
        auto modelPath = face::AndroidUtils::readStringUTF(env, imagePath);
        auto faceRecognizer = std::make_shared<face::RetinaFace>(modelPath);
        sImagePreviewer = std::make_shared<face::ImagePreviewer>(faceRecognizer);
        sImagePreviewer->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_jarvis_facekit_FaceKit_showImage(JNIEnv *env, jobject thiz, jstring image_path) {
    if (!sImagePreviewer) {
        return;
    }
    sImagePreviewer->requestLoadImage(face::AndroidUtils::readStringUTF(env, image_path));
}