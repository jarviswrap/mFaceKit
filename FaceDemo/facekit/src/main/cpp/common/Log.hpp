//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_LOG_HPP
#define FACEDEMO_LOG_HPP
#ifdef __ANDROID__
#include <android/log.h>
#ifndef LOG_TAG
#define LOG_TAG "VideoSystem"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif
#endif //FACEDEMO_LOG_HPP
