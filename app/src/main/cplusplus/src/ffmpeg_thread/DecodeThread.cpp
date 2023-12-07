/**
 * @file DecodeThread.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-12-07
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ffmpeg_thread/DecodeThread.h"
#include "log/Log.h"

using namespace mingzz;

#define DECODETHREAD_DEBUG_ON true

#define IF_DECODETHREAD_DEBUG_ON if( \
                 DECODETHREAD_DEBUG_ON \
                 )

DecodeThread::DecodeThread(AVPacketQueue*pktQueue,AVFrameQueue*frameQueue) :
                                          mpAVFQueue(frameQueue),
                                          mpAVPQueue(pktQueue)

{
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread(*,*)!");


}

DecodeThread::~DecodeThread(){
    IF_DECODETHREAD_DEBUG_ON LOGD("~DecodeThread()!");


}

void DecodeThread::stop(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

}

void DecodeThread::run(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

}

int DecodeThread::init(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

    return 0;
}