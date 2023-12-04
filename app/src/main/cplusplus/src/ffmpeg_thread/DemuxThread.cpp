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
                             mAPQueue(nullptr),
                             mVPQueue(nullptr),
                             mAVFContext(nullptr),
                             mAudioIndex(-1),
                             mVideoIndex(-1)
{
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread DemuxThread()!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));
}

DemuxThread::DemuxThread(AVPacketQueue* audioPQueue,AVPacketQueue*videoPQueue) : mAbort(0),
                                          mAPQueue(audioPQueue),
                                          mVPQueue(videoPQueue),
                                          mAVFContext(nullptr),
                                          mAudioIndex(-1),
                                          mVideoIndex(-1)
{
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread DemuxThread(AVPacketQueue* audioPQueue,AVPacketQueue*videoPQueue)!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));


}

DemuxThread::~DemuxThread() {
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread ~DemuxThread()!");
    stop();
}

void DemuxThread::stop() {
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread::stop()!");
    Thread::stop();
    mAbort = 1;
    if (mAVFContext) {
        avformat_close_input(&mAVFContext);
    }
    if(mVPQueue){
        delete mVPQueue;
        mVPQueue = nullptr;
    }
    if(mAPQueue){
        delete mAPQueue;
        mAPQueue = nullptr;
    }
}

void DemuxThread::run() {

    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread run()!");

    std::thread::id threadId = std::this_thread::get_id();
    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread Current thread id->%d", *(unsigned int *) &threadId);

    int ret = 0;
    AVPacket pkt;
    while (!mAbort) {

        if (!mAVFContext) {
            LOGE("DemuxThread run : mAVFContext is null");
            break;
        }

        ret = av_read_frame(mAVFContext, &pkt); //读取av packet
        if (ret < 0) {
            av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
            LOGE("DemuxThread run : mAVErrorInfo->%s", mAVErrorInfo);
            break;
        }
        if (mVideoIndex == pkt.stream_index) {
            // put into video pkt queue
            if(mVPQueue){
                IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread run : put video pkt, mVPQueue size->%d", mVPQueue->size());

                mVPQueue->push(&pkt);

            } else {
                LOGE("DemuxThread run : mVPQueue is nullptr");
            }

        } else if (mAudioIndex == pkt.stream_index) {
            // put into audio pkt queue
            if(mAPQueue){
                IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread run : put into audio pkt, mAPQueue size->%d", mAPQueue->size());

                mAPQueue->push(&pkt);

            } else {
                LOGE("DemuxThread run : mAPQueue is nullptr");
            }

        } else {
            //其他
            IF_DEMUXTHREAD_DEBUG_ON   LOGD("DemuxThread run : other stream index");

            av_packet_unref(&pkt); // 释放
        }

    }

    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread run : end!");
}

int DemuxThread::init(const char *url) {

    IF_DEMUXTHREAD_DEBUG_ON  LOGD("DemuxThread init : Url->%s!", url);

    mUrl = url;

    mAVFContext = avformat_alloc_context();
    if (!mAVFContext) {
        LOGE("DemuxThread init : mAVFContext is null");
        return -1;
    }

    int ret = avformat_open_input(&mAVFContext, mUrl.c_str(), nullptr, nullptr);
    if (ret < 0) {
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
        LOGE("DemuxThread init : avformat_open_input mAVErrorInfo --> %s", mAVErrorInfo);
        return ret;
    }

    ret = avformat_find_stream_info(mAVFContext, NULL);
    if (ret < 0) {
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
        LOGE("DemuxThread init : avformat_find_stream_info mAVErrorInfo --> %s", mAVErrorInfo);
        return ret;
    }

    av_dump_format(mAVFContext, 0, mUrl.c_str(), 0);

    mVideoIndex = av_find_best_stream(mAVFContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    mAudioIndex = av_find_best_stream(mAVFContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (mVideoIndex < 0 || mAudioIndex < 0) {
        // audio or video stream data not found!!
        LOGE("DemuxThread init : audio or video stream data not found!!");
        return -1;
    }

    IF_DEMUXTHREAD_DEBUG_ON LOGD("DemuxThread init : mVideoIndex->%d , mAudioIndex->%d", mVideoIndex, mAudioIndex);

    return 0;
}