#include <jni.h>
#include <string>
//#include "engine/FaceInference.hpp"
#include "android/AndroidUtils.hpp"
#include "common/Log.hpp"
#include "JNIEnvManager.hpp"

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
        jobject /* this */, jstring modelDir) {
    if (!modelDir) {
        return;
    }
//    if (!mInference) {
//        mInference = std::make_shared<face::FaceInference>();
//    }
//    mInference->init(face::AndroidUtils::readStringUTF(env, modelDir));
}