#include <jni.h>
#include <string>
#include "FFmpegLearning.h"
#include "Log.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/ffversion.h"
}

jstring FFmpegLearningActivity_stringFromJNI(JNIEnv *env, jobject jOject) {
    std::string info("FFmpegLearningActivity : \n");
    unsigned int avcodecVersionNumber = avcodec_version();
    LOGD("ffmpeg avcodecVersion-->%d(Version=[major:minor:micro]-->%d.%d.%d) ffmpegVersion-->%s ",
         avcodecVersionNumber,
         (avcodecVersionNumber >> 16) & 0xff,
         (avcodecVersionNumber >> 8) & 0xff,
         (avcodecVersionNumber) & 0xff,
         FFMPEG_VERSION);
    info += "ffmpeg version->" + std::string(FFMPEG_VERSION) + "\n" +
            "ffmpeg avcodecVersion->" + std::to_string(avcodecVersionNumber) + "\n" +
            "(Version=[major:minor:micro]-->" +
            std::to_string((avcodecVersionNumber >> 16) & 0xff) + "." +
            std::to_string((avcodecVersionNumber >> 8) & 0xff) + "." +
            std::to_string((avcodecVersionNumber) & 0xff) + "." +
            ")";
    return env->NewStringUTF(info.c_str());
}