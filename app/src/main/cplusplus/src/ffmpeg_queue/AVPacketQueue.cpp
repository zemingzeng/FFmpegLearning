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

#define AVPACKETQUEUE_DEBUG_ON true

#define IF_AVPACKTQUEUE_DEBUG_ON if( \
                 AVPACKETQUEUE_DEBUG_ON \
                 )

AVPacketQueue::AVPacketQueue(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue()!");
}

AVPacketQueue::~AVPacketQueue(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("~AVPacketQueue()!");

    //FIX ME : 需要完善
    while(true) {
         AVPacket* tmpPacket = nullptr;
         int ret = mQueue.pop(tmpPacket,1);
         if(0!=ret){
             LOGE("~AVPacketQueue queue pop : error ret->%d!",ret);
             break;
         }
         av_packet_free(&tmpPacket);
    }
}

void AVPacketQueue::abort(){
    IF_AVPACKTQUEUE_DEBUG_ON LOGD("AVPacketQueue abort!");

    mQueue.abort();
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
    if(mQueue.pop(tmpPacket,timeOut)){
        LOGE("AVPacketQueue pop : error happen!");
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