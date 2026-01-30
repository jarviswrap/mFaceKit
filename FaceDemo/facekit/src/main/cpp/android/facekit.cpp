#include <jni.h>
#include <string>
//#include "engine/FaceInference.hpp"
#include "android/AndroidUtils.hpp"
#include "common/Log.hpp"
#include "common/Rect.hpp"
#include "JNIEnvManager.hpp"
#include "android/example/ImagePreviewer.hpp"
#include "android/AndroidUtils.hpp"
#include "detect/RetinaFace.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"
#include "core/Composer.hpp"
#include "core/track/VideoTrack.hpp"
#include "core/clip/VideoClip.hpp"
#include "core/Timeline.hpp"

static std::shared_ptr<face::ImagePreviewer> sImagePreviewer = nullptr;
static std::shared_ptr<face::Timeline> sTimeline = nullptr;
static std::shared_ptr<face::Composer> sComposer = nullptr;

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
Java_com_jarvis_facekit_FaceKit_showVideo(JNIEnv *env, jobject thiz, jint trackId, jstring video_path, jlong start, jlong duration) {
    // TODO: implement showVideo()
    if (!sTimeline) {
        sTimeline = std::make_shared<face::Timeline>(true);
    }
    if (!sComposer) {
        sComposer = std::make_shared<face::Composer>();
        sComposer->init(sTimeline);
    }
    auto videoTrack = sComposer->getComponentById<face::VideoTrack>(trackId);
    if (!videoTrack) { // 不存在VideoTrack时直接创建
        videoTrack = std::make_shared<face::VideoTrack>();
        auto trackSize = sComposer->getTrackSize();
        trackId = sComposer->addTrack(face::Rect<float>(0.1f *(trackSize + 1), 0.1f * (trackSize + 1), 0.4f, 0.4f), videoTrack);
    }
    auto path = face::AndroidUtils::readStringUTF(env, video_path);
    LOGE("Java_com_jarvis_facekit_FaceKit_showVideo:%s", path.c_str());
    auto clip = std::make_shared<face::VideoClip>(path, videoTrack->getComponentId());
    if (start == -1) { // 从该track的上一个clip结束位置开始
//        start = videoTrack->getEnd();
        start = sTimeline->getCurrentTime();
    }
    if (duration == -1) { // 默认耗时10秒
        duration = 10000;
    }
    clip->start(start, start + duration); //默认10秒
    videoTrack->addClip(clip);
    return clip->getId();
}

extern "C"
JNIEXPORT void JNICALL
          Java_com_jarvis_facekit_FaceKit_tick(JNIEnv *env, jobject thiz) {
    if (sTimeline && sTimeline->isAutoTick()) {
        sTimeline->tick();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jarvis_facekit_FaceKit_touchVideo(JNIEnv *env, jobject thiz, jint videoClipId) {
    if (sComposer) {
        auto videoClip = sComposer->getComponentById<face::VideoClip>(videoClipId);
        if (videoClip) {
            auto videoTrackId = videoClip->getTrackId();
            sComposer->bringTrackToLast(videoTrackId);
        }
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_jarvis_facekit_FaceKit_showVideoAfter(JNIEnv *env,
                                               jobject thiz,
                                               jint video_id,
                                               jstring video_path) {
    if (sComposer) {
        auto videoClip = sComposer->getComponentById<face::VideoClip>(video_id);
        if (videoClip) {
            auto videoTrackId = videoClip->getTrackId();
            sComposer->bringTrackToLast(videoTrackId);
            auto videoTrack = sComposer->getComponentById<face::VideoTrack>(videoTrackId);
            auto path = face::AndroidUtils::readStringUTF(env, video_path);
            LOGE("Java_com_jarvis_facekit_FaceKit_showVideo:%s", path.c_str());
            auto clip = std::make_shared<face::VideoClip>(path, videoTrack->getComponentId());
            auto startTime = videoTrack->getEnd();
            clip->start(startTime, startTime + 10000); //默认10秒
            videoTrack->addClip(clip);
            return clip->getId();
        }
    }
    return 0;
}