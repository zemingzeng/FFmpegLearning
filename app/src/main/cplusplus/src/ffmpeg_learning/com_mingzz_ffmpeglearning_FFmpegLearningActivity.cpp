#include <jni.h>
#include <string>
#include "ffmpeg_learning/FFmpegLearning.h"
#include "log/Log.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/ffversion.h"
}

#include "safe_queue/SafeQueue.h"
#include "ffmpeg_thread/DemuxThread.h"

using namespace mingzz;

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
//    //test queue
//    SafeQueue<int> queue;
//    queue.push(1);
//    queue.push(2);
//    LOGD("queue size-->%d", queue.size());
//    queue.abort();
//    //test demux thread
//    DemuxThread *demuxThread = new DemuxThread();
//    demuxThread->init("xxx/xxx/xxx.mp4");
//    demuxThread->start();
//    demuxThread->join();
//    delete demuxThread;
    return env->NewStringUTF(info.c_str());
}

void FFmpegLearningActivity_ffmpegLearningStart(JNIEnv *env,jobject jOject,jstring url){

    const char *videoPath = env->GetStringUTFChars(url, nullptr);

    DemuxThread *demuxThread = new DemuxThread();
    demuxThread->init(videoPath);
    demuxThread->start();
    demuxThread->join();
    delete demuxThread;

    env->ReleaseStringUTFChars(url, videoPath);
}