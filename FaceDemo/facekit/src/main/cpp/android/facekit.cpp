#include <jni.h>
#include <string>
//#include "engine/FaceInference.hpp"
#include "android/AndroidUtils.hpp"
#include "common/Log.hpp"
#include "common/Rect.hpp"
#include "JNIEnvManager.hpp"
#include "android/example/ImagePreviewer.hpp"
#include "android/AndroidUtils.hpp"
#include "core/processor/RetinaFace.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"
#include "core/Composer.hpp"
#include "core/track/VideoTrack.hpp"
#include "core/clip/VideoClip.hpp"

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
extern "C"
JNIEXPORT void JNICALL
Java_com_jarvis_facekit_egl_EGLSurfaceView_setFaceLiftIntensity(JNIEnv *env,
                                                                jobject thiz,
                                                                jlong show_view_ptr,
                                                                jfloat intensity) {
    auto showView = face::EGLDelegate::getInstance().getShowView(show_view_ptr);
    if (showView) {
//        showView->setFaceListIntensity(intensity);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_jarvis_facekit_FaceKit_showVideo(JNIEnv *env, jobject thiz, jint trackIndex, jstring video_path) {
    // TODO: implement showVideo()
    static std::shared_ptr<face::Composer> sComposer = nullptr;
    if (!sComposer) {
        sComposer = std::make_shared<face::Composer>();
        sComposer->init();
    }
    auto track = sComposer->getTrackByIndex(trackIndex);
    if (!track || track->getType() != face::TrackType::Video) {
        track = std::make_shared<face::VideoTrack>();
        auto trackSize = sComposer->getTrackSize();
        trackIndex = sComposer->addTrack(face::Rect<float>(0.1f *(trackSize + 1), 0.1f * (trackSize + 1), 0.5f, 0.5f), track) - 1;
    }
    auto videoTrack = std::static_pointer_cast<face::VideoTrack>(track);
    videoTrack->addClip(std::make_shared<face::VideoClip>(face::AndroidUtils::readStringUTF(env, video_path)));
    return trackIndex + 1;
}