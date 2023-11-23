#include "ffmpeg_thread/DemuxThread.h"
#include "log/Log.h"
#include <iostream>

using namespace mingzz;

#define DEMUXTHREAD_DEBUG_ON true

DemuxThread::DemuxThread() : /* mAVFContext(nullptr)*/
        mAbort(0),
        mAudioIndex(-1),
        mVideoIndex(-1) {
    // memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));
}

DemuxThread::~DemuxThread() {
    stop();
}

void DemuxThread::stop() {
    Thread::stop();

    mAbort = 1;
    // if (mAVFContext)
    // {
    //     avformat_close_input(&mAVFContext);
    // }
}

void DemuxThread::run() {

    if (DEMUXTHREAD_DEBUG_ON)
        LOGD("DemuxThread run()!");

    std::thread::id threadId = std::this_thread::get_id();
    if (DEMUXTHREAD_DEBUG_ON)
        LOGD("DemuxThread Current thread id->%d", threadId);

    int ret = 0;
    // AVPacket pkt;
    while (mAbort) {
        // ret = av_read_frame(mAVFContext, &pkt); //读取av packet
        // if (ret < 0)
        // {
        //     av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
        //     break;
        // }
        // if (mVideoIndex == pkt.stream_index)
        // {
        //     // put into video pkt queue
        // }
        // else if (mAudioIndex == pkt.stream_index)
        // {
        //     // put into audio pkt queue
        // } else {
        // av_packet_unref(&pkt); // 释放
        // }
    }

    int count = 0;
    while (count < 5) {
        if (DEMUXTHREAD_DEBUG_ON)
            LOGD("DemuxThread running and count:%d", count);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        count++;
    }
}

int DemuxThread::init(const char *url) {

    if (DEMUXTHREAD_DEBUG_ON)
        LOGD("DemuxThread init  Url->%s!", url);

    mUrl = url;

    // mAVFContext = avformat_alloc_context();
    // if (!mAVFContext)
    // {
    // }

    // int ret = avformat_open_input(&mAVFContext, mUrl, nullptr, nullptr);
    // if (ret < 0)
    // {
    //     av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
    // }

    // ret = avformat_find_stream_info(mAVFContext, NULL);
    // if (ret < 0)
    // {
    //     av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo)); // 把错误码转化成string
    // }

    // av_dump_format(mAVFContext, 0, mUrl.c_str(), 0);

    // mVideoIndex = av_find_best_stream(mAVFContext, AVMEDIA_TPYE_VIDEO, -1, -1, nullptr, 0);
    // mAudioIndex = av_find_best_stream(mAVFContext, AVMEDIA_TPYE_AUDIO, -1, -1, nullptr, 0);
    if (mVideoIndex < 0 || mAudioIndex < 0) {
        // audio or video stream data not found!!
    }

    return 0;
}