#ifndef AVPLAY_ACTIVITY_H
#define AVPLAY_ACTIVITY_H

#include <jni.h>

void AVPlayActivity_startToPlay(JNIEnv *env,jobject jOject,
                                jstring url,
                                jobject surface);

void AVPlayActivity_stop();

#endif