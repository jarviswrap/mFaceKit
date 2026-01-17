//
// Created by wilbert on 2026/1/16.
//

#include "AndroidUtils.hpp"
#ifdef __ANDROID__
namespace face {
    jintArray AndroidUtils::createIntArray(JNIEnv *env, std::vector<int32_t> &arr) {
        jintArray array = nullptr;
        if (arr.empty() || !env) {
            return array;
        }
        auto size = (jsize)arr.size();
        array = env->NewIntArray(size);
        env->SetIntArrayRegion(array, 0, size, arr.data());
        return array;
    }

    std::vector<int32_t> AndroidUtils::readIntArray(JNIEnv *env, jintArray array) {
        std::vector<int32_t> result;
        if (!array) {
            return std::move(result);
        }
        jsize size = env->GetArrayLength(array);
        jint *_array = env->GetIntArrayElements(array, nullptr);
        if (size <= 0 || !_array) {
            return std::move(result);
        }
        result.resize(size);
        memcpy(&result[0], _array, size * sizeof(int32_t));
        return std::move(result);
    }

    std::string AndroidUtils::readStringUTF(JNIEnv *env, jstring str) {
        std::string result;
        if (!str) {
            return std::move(result);
        }
        const char *_str = env->GetStringUTFChars(str, nullptr);
        result.assign(_str);
        env->ReleaseStringUTFChars(str, _str);
        return std::move(result);
    }

    jstring AndroidUtils::readJStringUTF(JNIEnv *env, const std::string& str) {
        if (str.empty()) {
            return nullptr;
        }
        return env->NewStringUTF(str.c_str());
    }

    void AndroidUtils::vectorFloatToJArray(JNIEnv *env, const std::vector<float> &src, jobject obj,
                                           jfieldID fieldId) {
        auto jArray = (jfloatArray) env->GetObjectField(obj, fieldId);
        if (!jArray || env->GetArrayLength(jArray) != src.size()) {
            if (jArray) {
                env->DeleteLocalRef(jArray);
            }
            jArray = env->NewFloatArray((jsize)src.size());
            env->SetObjectField(obj, fieldId, jArray);
        }
        jfloat *array = env->GetFloatArrayElements(jArray, nullptr);
        std::copy(src.begin(), src.end(), array);
        env->ReleaseFloatArrayElements(jArray, array, 0);
        env->DeleteLocalRef(jArray);
    }

    void AndroidUtils::vectorIntToJArray(JNIEnv *env, const std::vector<int> &src, jobject obj,
                                         jfieldID fieldId) {
        auto jArray = (jintArray) env->GetObjectField(obj, fieldId);
        if (!jArray || env->GetArrayLength(jArray) != src.size()) {
            if (jArray) {
                env->DeleteLocalRef(jArray);
            }
            jArray = env->NewIntArray((jsize)src.size());
            env->SetObjectField(obj, fieldId, jArray);
        }
        jint *array = env->GetIntArrayElements(jArray, nullptr);
        std::copy(src.begin(), src.end(), array);
        env->ReleaseIntArrayElements(jArray, array, 0);
        env->DeleteLocalRef(jArray);
    }

    void
    AndroidUtils::vectorUInt32ToJArray(JNIEnv *env, const std::vector<uint32_t> &src, jobject obj,
                                       jfieldID fieldId) {
        auto jArray = (jintArray) env->GetObjectField(obj, fieldId);
        if (!jArray || env->GetArrayLength(jArray) != src.size()) {
            if (jArray) {
                env->DeleteLocalRef(jArray);
            }
            jArray = env->NewIntArray((jsize)src.size());
            env->SetObjectField(obj, fieldId, jArray);
        }
        jint *array = env->GetIntArrayElements(jArray, nullptr);
        std::copy(src.begin(), src.end(), array);
        env->ReleaseIntArrayElements(jArray, array, 0);
        env->DeleteLocalRef(jArray);
    }

    void
    AndroidUtils::vectorUint8ToJArray(JNIEnv *env, const std::vector<uint8_t> &src, jobject obj,
                                      jfieldID fieldId) {
        auto jArray = (jbyteArray) env->GetObjectField(obj, fieldId);
        if (!jArray || env->GetArrayLength(jArray) != src.size()) {
            if (jArray) {
                env->DeleteLocalRef(jArray);
            }
            jArray = env->NewByteArray((jsize)src.size());
            env->SetObjectField(obj, fieldId, jArray);
        }
        jbyte *array = env->GetByteArrayElements(jArray, nullptr);
        std::copy(src.begin(), src.end(), array);
        env->ReleaseByteArrayElements(jArray, array, 0);
        env->DeleteLocalRef(jArray);
    }

} // face
#endif