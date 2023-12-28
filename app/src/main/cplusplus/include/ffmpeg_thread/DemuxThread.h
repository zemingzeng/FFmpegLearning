/**
 * @file DemuxThread.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include "thread/Thread.h"
#include "ffmpeg_queue/AVPacketQueue.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

namespace mingzz {

    class DemuxThread : public Thread {

    public:
        DemuxThread();
        DemuxThread(AVPacketQueue*,AVPacketQueue*);
        ~DemuxThread();
        int init(const char *);
        void stop();

    public:
        AVCodecParameters*  getAudioCodecParams();
        AVCodecParameters*  getVideoCodecParams();
        AVRational  getAVStreamTimeBase();

    private:
        void run() override;

    private:
        int mAbort;
        char mAVErrorInfo[256];
        std::string mUrl;
        AVPacketQueue *mpVPQueue; // video packet queue
        AVPacketQueue *mpAPQueue; // audio packet queue
        AVFormatContext *mpAVFContext;
        int mAudioIndex;
        int mVideoIndex;
    };
}

#endif