#ifndef FFMPEG_LEARNING_H
#define FFMPEG_LEARNING_H

#include <jni.h>

jstring FFmpegLearningActivity_stringFromJNI(JNIEnv *env,jobject jOject);

void FFmpegLearningActivity_ffmpegLearningStart(JNIEnv *env,jobject jOject,jstring url);

#endif