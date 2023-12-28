#ifndef AUDIO_THREAD_H
#define AUDIO_THREAD_H

#include "thread/Thread.h"
#include "ffmpeg_queue/AVFrameQueue.h"

extern "C" {
//ffmpeg
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
//opensl es
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>
}

#include <stdint.h>

using namespace mingzz;

class AudioThread : public Thread{

public :

    AudioThread(AVFrameQueue*,
                int sampleRate,
                enum AVSampleFormat,
                AVChannelLayout);
    ~AudioThread();
    int init();
    int prepareFrameBuffer();
    int stop();

private :

     virtual void run();
     void prepareOpenSL();
     void prepareSwrContext();
     uint32_t createOpenSLSampleRate(int rate);
     AVFrameQueue* mpAVFrameQueue;
     SwrContext *mpSwrContext;
     int mSampleRate;
     enum AVSampleFormat mSampleFormat;
     AVChannelLayout mAVChannelLayout;
     int mAbort;
     bool mPrepared;
     //opensl
     SLObjectItf mpSLEngineObject;
     SLObjectItf mpSLMixerObject;
     SLObjectItf mpSLAudioPlayer;
};

#endif