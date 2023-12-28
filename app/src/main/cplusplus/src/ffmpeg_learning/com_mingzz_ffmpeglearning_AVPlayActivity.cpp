#include "ffmpeg_learning/AVPlayActivity.h"
#include "log/Log.h"
#include "ffmpeg_learning/VideoThread.h"
#include "ffmpeg_learning/AudioThread.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/ffversion.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
}

//#include "safe_queue/SafeQueue.h"
#include "ffmpeg_thread/DemuxThread.h"
#include "ffmpeg_thread/DecodeThread.h"
#include "ffmpeg_queue/AVPacketQueue.h"
#include "ffmpeg_queue/AVFrameQueue.h"

#include <string>
#include <android/native_window_jni.h>
#include <stdint.h>

using namespace mingzz;

ANativeWindow* nativeWindow = nullptr;
uint8_t* forWindowFrameBuffer = nullptr;
SwsContext* swsContext = nullptr;

AVPacketQueue *pAudioPQueue = nullptr;
AVPacketQueue *pVideoPQueue = nullptr;

AVFrameQueue *pAudioFQueue = nullptr;
AVFrameQueue *pVideoFQueue = nullptr;

DemuxThread* pDemuxThread = nullptr;
DecodeThread* pAudioDecodeThread = nullptr;
DecodeThread* pVideoDecodeThread = nullptr;

VideoThread* pVideoThread = nullptr;
AudioThread* pAudioThread = nullptr;

void prepareWindow(JNIEnv *env,
                   int videoStreamWidth,
                   int videoStreamHeight,
                   jobject surface);

//stop thread and release resources
void AVPlayActivity_stop(){
    if(pVideoThread){
        pVideoThread->stop();
    }
    if(pAudioThread){
        pAudioThread->stop();
    }
    if(pAudioDecodeThread){
        pAudioDecodeThread->stop();
    }
    if(pVideoDecodeThread){
        pVideoDecodeThread->stop();
    }
    if(pDemuxThread){
        pDemuxThread->stop();
    }
    if(pAudioFQueue){
    }
    if(pVideoFQueue){
    }
    if(pAudioPQueue){
    }
    if(pVideoFQueue){
    }
    if(swsContext){
        sws_freeContext(swsContext);
    }
    //thread
    delete pDemuxThread;
    pDemuxThread = nullptr;
    delete pAudioDecodeThread;
    pAudioDecodeThread = nullptr;
    delete pVideoDecodeThread;
    pVideoDecodeThread = nullptr;
    delete pVideoThread;
    pVideoThread = nullptr;
    delete pAudioThread;
    pAudioThread = nullptr;
    //queue
    //std::this_thread::sleep_for(std::chrono::milliseconds(50000));
    delete pAudioFQueue;
    pAudioFQueue = nullptr;
    delete pVideoFQueue;
    pVideoFQueue = nullptr;
    delete pAudioPQueue;
    pAudioPQueue = nullptr;
    delete pVideoPQueue;
    pVideoPQueue = nullptr;
}


void AVPlayActivity_startToPlay(JNIEnv *env, jobject jOject,
                                jstring url,
                                jobject surface){

    LOGD("AVPlayActivity AVPlayActivity_startToPlay : begin");

    const char *videoPath = env->GetStringUTFChars(url, nullptr);

    pAudioPQueue = new AVPacketQueue;
    pVideoPQueue = new AVPacketQueue;

    pAudioFQueue = new AVFrameQueue;
    pVideoFQueue = new AVFrameQueue;

    pDemuxThread = new DemuxThread(pAudioPQueue, pVideoPQueue);
    pAudioDecodeThread = new DecodeThread(pAudioPQueue, pAudioFQueue);
    pVideoDecodeThread = new DecodeThread(pVideoPQueue, pVideoFQueue);


    int ret = pDemuxThread->init(videoPath);
     if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : demuxThread init error->%d",ret);
        return;
    }

    ret = pAudioDecodeThread->init(pDemuxThread->getAudioCodecParams());
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : audioDecodeThread init error->%d",ret);
        return;
    }

    ret = pVideoDecodeThread->init(pDemuxThread->getVideoCodecParams());
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : videoDecodeThread init error->%d",ret);
        return;
    }

    //get native window(surface) after decode thread init
    prepareWindow(env,
                  pVideoDecodeThread->getVideoWidthFromCodecContext(),
                  pVideoDecodeThread->getVideoHeightFromCodecContext(),
                  surface);

    //video render
    pVideoThread = new VideoThread(
            nativeWindow,
            pVideoFQueue,
            pDemuxThread->getAVStreamTimeBase(),
            pVideoDecodeThread->getVideoPixFormatFromCodecContext(),
            pVideoDecodeThread->getVideoWidthFromCodecContext(),
            pVideoDecodeThread->getVideoHeightFromCodecContext());

    ret = pVideoThread->init();
    if(0 != ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : pVideoThread init fail!");
    }

    pAudioThread = new AudioThread(pAudioFQueue,
                                                pAudioDecodeThread->getAudioSampleRateFormCodecContext(),
                                                pAudioDecodeThread->getAudioSampleFormatFormCodecContext(),
                                                pAudioDecodeThread->getAudioChannelLayoutFormCodecContext());

    ret = pAudioThread->init();
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : audioThread init error->%d",ret);
        return;
    }

    ret = pDemuxThread->start();
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : demuxThread start error->%d",ret);
        return;
    }

    ret = pAudioDecodeThread->start();
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : audioDecodeThread start error->%d",ret);
        return;
    }

    ret = pVideoDecodeThread->start();
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : videoDecodeThread start error->%d",ret);
        return;
    }

    ret = pVideoThread->start();
    if(0 > ret){
        LOGE("AVPlayActivity AVPlayActivity_startToPlay : videoThread start error->%d",ret);
        return;
    }

    env->ReleaseStringUTFChars(url, videoPath);
    LOGD("AVPlayActivity AVPlayActivity_startToPlay : end");
    
}

void prepareWindow(JNIEnv *env, int videoStreamWidth, int videoStreamHeight, jobject surface){
    //surface(cpp) inherit from ANativeWindow
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    LOGD("AVPlayActivity prepareWindow : video width : height %d : %d",videoStreamWidth,videoStreamHeight);

    //set the format and size of the window buffers.
    //native window only support :
    //WINDOW_FORMAT_RGBA_8888
    //WINDOW_FORMAT_RGBX_8888
    //WINDOW_FORMAT_RGB_565
    //RGBA_8888->Red: 8 bits, Green: 8 bits, Blue: 8 bits, Alpha: 8 bits.
    int ret = ANativeWindow_setBuffersGeometry(nativeWindow,
                                               videoStreamWidth,
                                               videoStreamHeight,
                                               WINDOW_FORMAT_RGBA_8888);
    if(0 != ret){
        LOGE("AVPlayActivity prepareWindow : ANativeWindow_setBuffersGeometry fail!");
    }
}