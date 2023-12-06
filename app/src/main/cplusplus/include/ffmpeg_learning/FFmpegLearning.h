/**
 * @file FFmpegLearning.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef FFMPEG_LEARNING_H
#define FFMPEG_LEARNING_H

#include <jni.h>

jstring FFmpegLearningActivity_stringFromJNI(JNIEnv *env,jobject jOject);

void FFmpegLearningActivity_ffmpegLearningStart(JNIEnv *env,jobject jOject,jstring url);

#endif