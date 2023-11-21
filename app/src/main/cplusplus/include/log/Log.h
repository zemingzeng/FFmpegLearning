/**
 * @file Log.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-21
 *
 * @copyright Copyright (c) 2023
 *
 */


#ifndef NATIVE_LOG_H
#define NATIVE_LOG_H

#include <jni.h>
#include <android/log.h>

#define NATIVE_LOG_DEBUG true

#define LOGI(...) \
if (NATIVE_LOG_DEBUG)        \
__android_log_print(ANDROID_LOG_INFO,"mingzz__jni",__VA_ARGS__)

#define LOGD(...) \
if (NATIVE_LOG_DEBUG)        \
__android_log_print(ANDROID_LOG_DEBUG,"mingzz__jni",__VA_ARGS__)

#define LOGW(...) \
if(NATIVE_LOG_DEBUG) \
__android_log_print(ANDROID_LOG_WARN,"mingzz__jni",__VA_ARGS__)

#define LOGE(...) \
if(NATIVE_LOG_DEBUG)         \
__android_log_print(ANDROID_LOG_ERROR,"mingzz__jni",__VA_ARGS__)

#endif