#include <jni.h>
#include <string>
#include "FFmpegLearning.h"
#include "Log.h"

jstring FFmpegLearningActivity_stringFromJNI(JNIEnv *env,jobject jOject){
    std::string info("FFmpegLearningActivity show!!!!!");
    return  env->NewStringUTF(info.c_str());
}