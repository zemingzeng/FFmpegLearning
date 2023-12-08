/**
 * @file DecodeThread.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-12-07
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include"thread/Thread.h"
#include"ffmpeg_queue/AVPacketQueue.h"
#include"ffmpeg_queue/AVFrameQueue.h"

extern "C"{
#include "libavcodec/avcodec.h"
};

namespace mingzz {

class DecodeThread : public Thread{

public:

    DecodeThread(AVPacketQueue*,AVFrameQueue*);

    ~DecodeThread();

    int stop();

    int init(AVCodecParameters*);

private:

    void run() override;

    AVFrameQueue* mpAVFQueue;

    AVPacketQueue* mpAVPQueue;

    AVCodecContext* mpAVCodecContext;

    char mAVErrorInfo[256];

};

}

#endif
