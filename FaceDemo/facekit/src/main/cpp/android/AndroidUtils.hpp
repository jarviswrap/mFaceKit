//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_ANDROIDUTILS_HPP
#define FACEDEMO_ANDROIDUTILS_HPP
#ifdef __ANDROID__
#include <string>
#include <jni.h>
#include <vector>
#include <string>
namespace face {

    class AndroidUtils {
    public:
        static jintArray createIntArray(JNIEnv *env, std::vector<int32_t> &param);
        static std::vector<int32_t> readIntArray(JNIEnv *env, jintArray array);
        static std::string readStringUTF(JNIEnv *env, jstring str);
        static jstring readJStringUTF(JNIEnv *env, const std::string &str);

        //vector数据拷贝到对象obj的fieldId字段
        static void vectorFloatToJArray(JNIEnv *env,
                                        const std::vector<float> &src,
                                        jobject obj,
                                        jfieldID fieldId);
        static void
        vectorIntToJArray(JNIEnv *env, const std::vector<int> &src, jobject obj, jfieldID fieldId);
        static void vectorUInt32ToJArray(JNIEnv *env,
                                         const std::vector<uint32_t> &src,
                                         jobject obj,
                                         jfieldID fieldId);
        static void vectorUint8ToJArray(JNIEnv *env,
                                        const std::vector<uint8_t> &src,
                                        jobject obj,
                                        jfieldID fieldId);
    };

} // face
#endif
#endif //FACEDEMO_ANDROIDUTILS_HPP
