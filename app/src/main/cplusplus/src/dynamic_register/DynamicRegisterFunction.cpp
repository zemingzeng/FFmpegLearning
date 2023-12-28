/**
 * @file DynamicRegisterFunction.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <jni.h>
#include <string>
#include "log/Log.h"
#include "ffmpeg_learning/FFmpegLearning.h"
#include "ffmpeg_learning/AVPlayActivity.h"

/***********************************************************************************************************
 * register native methods
 * ********************************************************************************************************/

/**
 * example:
 * Class and package name
 * com.mingzz__.h26x.rtmp.RTMPActivity.java
 * static const char *classPathName = "com/mingzz__/h26x/usta/rtmp/RTMPActivity";
 */
static const char *classPathName_1 = "com/mingzz/ffmpeglearning/FFmpegLearningActivity";
static const char *classPathName_2 = "com/mingzz/ffmpeglearning/AVPlayActivity";

/**
 * example:
 *
 * List of native methods
 *
 *   //void startCalibration() :
 *  {"startCalibration" , "()V", (void *)startCalibration},
 *
 *  //String getAccelCalFromNative() :
 *  {"getAccelCalFromNative" , "()Ljava/lang/String;", (void *)getAccelCalFromNative},
 */
static JNINativeMethod methods_1[] = {
        {"stringFromJNI", "(III)Ljava/lang/String;", (void *)FFmpegLearningActivity_stringFromJNI},
};
static JNINativeMethod methods_2[] = {
        {"avStartToPlay", "(Ljava/lang/String;Landroid/view/Surface;)V", (void *)AVPlayActivity_startToPlay},
        {"avStop", "()V", (void *)AVPlayActivity_stop},
};

/**
* Register several native methods for one class.
**/
static int
registerNativeMethods(JNIEnv *envVar, const char *inClassName, JNINativeMethod *inMethodsList,
                      int inNumMethods) {
    jclass javaClazz = envVar->FindClass(inClassName);
    if (javaClazz == NULL) {
        return JNI_FALSE;
    }
    if (envVar->RegisterNatives(javaClazz, inMethodsList, inNumMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/**
* Register native methods for all classes we know about.
* Returns JNI_TRUE on success
* */
static int registerNatives(JNIEnv *env) {
    if (!registerNativeMethods(env, classPathName_1, methods_1,
                               sizeof(methods_1) / sizeof(methods_1[0]))) {
        LOGE("registerNatives fail : %s", classPathName_1);
        return JNI_FALSE;
    }
    if (!registerNativeMethods(env, classPathName_2, methods_2,
                               sizeof(methods_2) / sizeof(methods_2[0]))) {
        LOGE("registerNatives fail : %s", classPathName_2);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

typedef union {
    JNIEnv *env;
    void *venv;
} UnionJNIEnvToVoid;

/**
* This is called by the VM when the shared library is first loaded.
*/
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    JNIEnv *env = NULL;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        return -1;
    }
    LOGI("JNI_OnLoad Register Natives Methods Success!!!");
    return JNI_VERSION_1_6;
}