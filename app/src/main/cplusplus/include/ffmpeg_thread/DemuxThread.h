#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include "thread/Thread.h"
#include "ffmpeg_queue/AVPacketQueue.h"

extern "C" {
#include "libavformat/avformat.h"
};

namespace mingzz {

    class DemuxThread : public Thread {
    public:
        DemuxThread();

        DemuxThread(AVPacketQueue*,AVPacketQueue*);

        ~DemuxThread();

        int init(const char *);

        void stop();

    private:
        void run() override;

    private:
        int mAbort;
        char mAVErrorInfo[256];
        std::string mUrl;
        AVPacketQueue *mVPQueue; // video packet queue
        AVPacketQueue *mAPQueue; // audio packet queue
        AVFormatContext *mAVFContext;
        int mAudioIndex;
        int mVideoIndex;
    };
}

#endif