#include <jni.h>
#include <string>
#include "ffmpeg_learning/FFmpegLearning.h"
#include "log/Log.h"
#include <thread>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/ffversion.h"
}

//#include "safe_queue/SafeQueue.h"
#include "ffmpeg_thread/DemuxThread.h"
#include "ffmpeg_thread/DecodeThread.h"
#include "ffmpeg_queue/AVPacketQueue.h"
#include "ffmpeg_queue/AVFrameQueue.h"

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
    LOGD("FFmpegLearningActivity_ffmpegLearningStart : begin");

    const char *videoPath = env->GetStringUTFChars(url, nullptr);

    AVPacketQueue audioPQueue ;
    AVPacketQueue videoPQueue;

    AVFrameQueue audioFQueue;
    AVFrameQueue videoFQueue;

    DemuxThread *demuxThread = new DemuxThread(&audioPQueue, &videoPQueue);
    DecodeThread* audioDecodeThread = new DecodeThread(&audioPQueue, &audioFQueue);
    DecodeThread* videoDecodeThread = new DecodeThread(&videoPQueue, &videoFQueue);

    int ret = demuxThread->init(videoPath);
     if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : demuxThread init error->%d",ret);
        return;
    }

    ret = audioDecodeThread->init(demuxThread->getAudioCodecParams());
    if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : audioDecodeThread init error->%d",ret);
        return;
    }

    ret = videoDecodeThread->init(demuxThread->getVideoCodecParams());
    if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : videoDecodeThread init error->%d",ret);
        return;
    }

    ret = demuxThread->start();
    if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : demuxThread start error->%d",ret);
        return;
    }

    ret = audioDecodeThread->start();
    if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : audioDecodeThread start error->%d",ret);
        return;
    }

    ret = videoDecodeThread->start();
    if(0 > ret){
        LOGE("FFmpegLearningActivity_ffmpegLearningStart : videoDecodeThread start error->%d",ret);
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    demuxThread->stop();
    delete demuxThread;

    audioDecodeThread->stop();
    delete audioDecodeThread;

    videoDecodeThread->stop();
    delete videoDecodeThread;

    env->ReleaseStringUTFChars(url, videoPath);
    LOGD("FFmpegLearningActivity_ffmpegLearningStart : end");
}