/**
 * @file AVPacketQueue.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ffmpeg_queue/AVPacketQueue.h"
#include "log/Log.h"

extern "C"{
    #include "libavcodec/codec.h"
}

using namespace mingzz;

#define AVPACKETQUEUE_DEBUG_ON true

#define IF_AVPACKTQUEUE_DEBUG_ON if( \
                 AVPACKETQUEUE_DEBUG_ON \
                 )

AVPacketQueue::AVPacketQueue(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue()!");
}

AVPacketQueue::~AVPacketQueue(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("~AVPacketQueue()!");

    abort();
}

void AVPacketQueue::release(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue release!");

    while(true) {
        AVPacket* tmpPacket = nullptr;
        int ret = mQueue.pop(tmpPacket,1);
        if(0!=ret){
            LOGW("AVPacketQueue queue pop : may fail!,queue size->%d!",mQueue.size());
            break;
        }
        av_packet_free(&tmpPacket);
    }

    mQueue.abort();
}

void AVPacketQueue::abort(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue abort!");

    release();
}

int AVPacketQueue::push(AVPacket* packet){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue push!");

    //finally must be freed using av_packet_free()
    AVPacket* tmpPacket = av_packet_alloc();
    // just copy and reset src packet
    av_packet_move_ref(tmpPacket,packet);
    // queue中保存的是拷贝份
    return mQueue.push(tmpPacket);
}

AVPacket* AVPacketQueue::pop(int timeOut){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue pop : timeOut->%d!",timeOut);

    if(0 > timeOut){
        LOGE("AVFrameQueue pop : error->timeout < 0");
    }

    AVPacket* tmpPacket = nullptr;
    if(0 != mQueue.pop(tmpPacket,timeOut)){
        LOGE("AVPacketQueue pop : error happen mQueue size->%d", mQueue.size());
    }

    return tmpPacket;
}

 AVPacket* AVPacketQueue::front(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue front!");

    AVPacket* tmpPacket = nullptr;
    if(mQueue.front(tmpPacket)){
        LOGE("AVPacketQueue front : error happen!");
    }

    return tmpPacket;
}

int AVPacketQueue::size(){
    int size = mQueue.size();

    //IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue size : size->%d!",size);

    return size;
}