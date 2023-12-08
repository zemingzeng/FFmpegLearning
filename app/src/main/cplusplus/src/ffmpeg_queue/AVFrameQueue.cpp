/**
 * @file AVFrameQueue.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ffmpeg_queue/AVFrameQueue.h"
#include "log/Log.h"

using namespace mingzz;

#define AVFRAMEQUEUE_DEBUG_ON true

#define IF_AVFRAMEQUEUE_DEBUG_ON if( \
                 AVFRAMEQUEUE_DEBUG_ON \
                 )

AVFrameQueue::AVFrameQueue(){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue()!");
}

AVFrameQueue::~AVFrameQueue(){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("~AVFrameQueue()!");

    abort();
}

void AVFrameQueue::release(){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue release!");

    while(true) {
        AVFrame* tmpFrame = nullptr;
        int ret = mQueue.pop(tmpFrame,1);
        if(0!=ret){
            LOGW("AVFrameQueue queue pop : may fail or no item to get ret->%d!",ret);
            break;
        }
        av_frame_free(&tmpFrame);
    }

}

void AVFrameQueue::abort(){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue abort");

    release();

    mQueue.abort();
}

int AVFrameQueue::size(){

    int size = mQueue.size();
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue  size : ->%d",size);

    return size;
}

int AVFrameQueue::push(AVFrame* frame){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue push : ->%p" ,frame);

    AVFrame* tmpFrame = av_frame_alloc();
    av_frame_move_ref(tmpFrame,frame);

    return mQueue.push(tmpFrame);
}

AVFrame* AVFrameQueue::pop(const int timeOut){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue pop ");

    if(0 > timeOut){
        LOGE("AVFrameQueue pop : error->timeout < 0");
    }

    AVFrame* tmpFrame = nullptr;
    if(0 != mQueue.pop(tmpFrame,timeOut)){
        LOGE("AVFrameQueue pop : error happen");
    }

    return tmpFrame;
}

AVFrame* AVFrameQueue::front(){
    IF_AVFRAMEQUEUE_DEBUG_ON LOGD("AVFrameQueue front ");

    AVFrame* tmpFrame = nullptr;
    if(0 != mQueue.front(tmpFrame)){
        LOGE("AVFrameQueue front : error happen");
    }

    return tmpFrame;
}
