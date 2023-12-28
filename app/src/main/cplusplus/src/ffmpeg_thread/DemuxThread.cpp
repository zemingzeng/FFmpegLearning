/**
 * @file DemuxThread.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ffmpeg_thread/DemuxThread.h"
#include "log/Log.h"
#include <iostream>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/error.h"
#include "libavutil/avutil.h"
}

using namespace mingzz;

#define DEMUXTHREAD_DEBUG_ON true

#define IF_DEMUXTHREAD_DEBUG_ON if( \
                 DEMUXTHREAD_DEBUG_ON \
                 )

DemuxThread::DemuxThread() : mAbort(0),
                             mpAPQueue(nullptr),
                             mpVPQueue(nullptr),
                             mpAVFContext(nullptr),
                             mAudioIndex(-1),
                             mVideoIndex(-1)
{
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread DemuxThread()!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));
}

DemuxThread::DemuxThread(AVPacketQueue* audioPQueue,AVPacketQueue*videoPQueue) : mAbort(0),
                                          mpAPQueue(audioPQueue),
                                          mpVPQueue(videoPQueue),
                                          mpAVFContext(nullptr),
                                          mAudioIndex(-1),
                                          mVideoIndex(-1)
{
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread DemuxThread(AVPacketQueue* audioPQueue,AVPacketQueue*videoPQueue)!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));

}

DemuxThread::~DemuxThread() {
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread ~DemuxThread()!");

    stop();

    if (mpAVFContext) {
        avformat_close_input(&mpAVFContext);
    }
}

void DemuxThread::stop() {
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread::stop()!");

    mAbort = 1;

    Thread::stop();

}

void DemuxThread::run() {

    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread run()!");

    std::thread::id threadId = std::this_thread::get_id();
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread Current thread id->%d", *(unsigned int *) &threadId);

    int ret = 0;
    AVPacket pkt;
    while (1!=mAbort) {

        //wait for av packet consumed
        //if(100 < mpVPQueue->size()){
        //if(100 < mpAPQueue->size()){
        if(10 < mpVPQueue->size() || 20 < mpAPQueue->size()){
            IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread run : wait for av packet consumed!");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (!mpAVFContext) {
            LOGE("DemuxThread run : mAVFContext is null");
            break;
        }

        //读取av packet
        //把avbuffer赋值给pkt（之前以为会copy一份赋值好buffer数据的packet）,
        //然后初始化refcount为1，
        //然后通过move_ref copy（转移）到queue中，所以在这不需要unref
        //当从queue中取出packet，用完时在free(会先unref后free)
        ret = av_read_frame(mpAVFContext, &pkt);
        if (ret < 0) {
            av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
            LOGE("DemuxThread run : mAVErrorInfo->%s", mAVErrorInfo);
            break;
        }
        if (mVideoIndex == pkt.stream_index) {
            // put into video pkt queue
            if(mpVPQueue){
                IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread run : put video pkt, mVPQueue size->%d",
                                              mpVPQueue->size());

                ret = mpVPQueue->push(&pkt);
                if(0 != ret){
                   LOGE(" DemuxThread run : video packet push error!");
                }

            } else {
                LOGE("DemuxThread run : mVPQueue is nullptr");
            }

        } else if (mAudioIndex == pkt.stream_index) {
            // put into audio pkt queue
            if(mpAPQueue){
                IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread run : put audio pkt, mAPQueue size->%d", mpAPQueue->size());

                ret = mpAPQueue->push(&pkt);
                if(0 != ret){
                   LOGE(" DemuxThread run : audio packet push error!");
                }

            } else {
                LOGE("DemuxThread run : mAPQueue is nullptr");
            }

        } else {
            //其他
            IF_DEMUXTHREAD_DEBUG_ON   LOGD("DemuxThread run : other stream index");

            av_packet_unref(&pkt); // 不需要这个buf,解除buf的引用
        }

    }

    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread run : end!");
}

int DemuxThread::init(const char *url) {

    IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread init : Url->%s!", url);

    mUrl = url;

    mpAVFContext = avformat_alloc_context();
    if (!mpAVFContext) {
        LOGE("DemuxThread init : mAVFContext is null");
        return -1;
    }

    int ret = avformat_open_input(&mpAVFContext, mUrl.c_str(), nullptr, nullptr);
    if (ret < 0) {
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
        LOGE("DemuxThread init : avformat_open_input mAVErrorInfo --> %s", mAVErrorInfo);
        return ret;
    }

    ret = avformat_find_stream_info(mpAVFContext, NULL);
    if (ret < 0) {
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
        LOGE("DemuxThread init : avformat_find_stream_info mAVErrorInfo --> %s", mAVErrorInfo);
        return ret;
    }

    //android platform print nothing!
    //av_dump_format(mAVFContext, 0, mUrl.c_str(), 0);

    mVideoIndex = av_find_best_stream(mpAVFContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    mAudioIndex = av_find_best_stream(mpAVFContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (mVideoIndex < 0 || mAudioIndex < 0) {
        // audio or video stream data not found!!
        LOGE("DemuxThread init : audio or video stream data not found!!");
        return -1;
    }

    AVStream* videoStream = mpAVFContext->streams[mVideoIndex];
    AVStream* audioStream = mpAVFContext->streams[mVideoIndex];
    LOGD("DemuxThread init : videoStream time_base(num->%d,den->%d) , audioStream time_base(num->%d,den->%d)",
         videoStream->time_base.num, videoStream->time_base.den,
         audioStream->time_base.num, audioStream->time_base.den);


    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread init : finish (mVideoIndex->%d , mAudioIndex->%d)",
                                  mVideoIndex, mAudioIndex);

    return 0;
}

AVCodecParameters*  DemuxThread::getAudioCodecParams(){
    AVCodecParameters* pParams = nullptr;
    AVStream* pStream = mpAVFContext->streams[mAudioIndex];
    pParams = pStream->codecpar;
    return pParams;
 }

AVCodecParameters*  DemuxThread::getVideoCodecParams(){
    AVCodecParameters* pParams = nullptr;
    AVStream* pStream = mpAVFContext->streams[mVideoIndex];
    pParams = pStream->codecpar;
    return pParams;
}

AVRational  DemuxThread::getAVStreamTimeBase(){
    //suppose audio video time_base is the same
    if(!mpAVFContext && -1 == mVideoIndex){
        LOGE("DemuxThread getAVStreamTimeBase : invalid status!");
    }
    return mpAVFContext->streams[mVideoIndex]->time_base;
}