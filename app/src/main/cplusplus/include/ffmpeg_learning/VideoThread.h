#ifndef  VIDEO_THREAD_H
#define  VIDEO_THREAD_H

#include "thread/Thread.h"
#include "ffmpeg_queue/AVFrameQueue.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
}

#include <android/native_window_jni.h>

using namespace mingzz;

class VideoThread : public Thread{
public:
    VideoThread();
    VideoThread(ANativeWindow*,
                AVFrameQueue*,
                AVRational timeBase,
                enum AVPixelFormat mVideoStreamFormat,
                int videoStreamWidth,
                int videoStreamHeight
                );
    ~VideoThread();
    int init();
    int stop();
private:
    virtual void run();
    int prepareSWSContextAndRGBAFrame(int videoStreamWidth,
                                   int videoStreamHeight,
                                   enum AVPixelFormat videoStreamFormat);
    void refreshForNativeWindow(AVFrame*);
    ANativeWindow* mpNativeWindow;
    AVFrameQueue* mpAVFrameQueue;
    SwsContext* mpSwsContext;
    uint8_t* mpForWindowFrameBuffer;
    AVFrame* mpNativeWindowRGBAFrame;
    ANativeWindow_Buffer mNativeWindowBuffer;
    AVRational mTimeBase;
    enum AVPixelFormat mVideoStreamFormat;
    int mVideoStreamWidth;
    int mVideoStreamHeight;
    int mAbort;
    bool mPrepared;
    uint64_t refreshStartTime;

};

#endif