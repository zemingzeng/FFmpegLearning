#include "ffmpeg_learning/VideoThread.h"

extern "C" {
#include "libavutil/imgutils.h"
}

#include <stdint.h>
#include <math.h>

VideoThread::VideoThread(
        ) :
        mpNativeWindow(nullptr),
        mpAVFrameQueue(nullptr),
        mpSwsContext(nullptr),
        mpForWindowFrameBuffer(nullptr),
        mpNativeWindowRGBAFrame(nullptr),
        mTimeBase{0,1},
        mVideoStreamFormat(AV_PIX_FMT_NONE),
        mVideoStreamWidth(-1),
        mVideoStreamHeight(-1),
        mAbort(0),
        mPrepared(false),
        refreshStartTime(0)
{
}

VideoThread::~VideoThread()
{
    stop();
    if(mpSwsContext)
        sws_freeContext(mpSwsContext);
    if(mpForWindowFrameBuffer)
        av_free(mpForWindowFrameBuffer);
    if(mpNativeWindowRGBAFrame)
        av_frame_free(&mpNativeWindowRGBAFrame);
}

VideoThread::VideoThread(ANativeWindow* nativeWindow,
                AVFrameQueue* avFrameQueue,
                AVRational timeBase,
                enum AVPixelFormat videoStreamFormat,
                int videoStreamWidth,
                int videoStreamHeight
                ):
                mpNativeWindow(nativeWindow),
                mpAVFrameQueue(avFrameQueue),
                mpSwsContext(nullptr),
                mpForWindowFrameBuffer(nullptr),
                mpNativeWindowRGBAFrame(nullptr),
                mTimeBase(timeBase),
                mVideoStreamFormat(videoStreamFormat),
                mVideoStreamWidth(videoStreamWidth),
                mVideoStreamHeight(videoStreamHeight),
                mAbort(0),
                mPrepared(false),
                refreshStartTime(0)
{
    //LOGD("VideoThread constructor : mVideoStreamWidth->%d,mVideoStreamHeight->%d",
         //mVideoStreamWidth,mVideoStreamHeight);
}

int VideoThread::init(){
    int ret = prepareSWSContextAndRGBAFrame(mVideoStreamWidth,
                                            mVideoStreamHeight,
                                            mVideoStreamFormat);
    mPrepared = (nullptr == mpAVFrameQueue || nullptr == mpNativeWindow ) ? false : true;
    return ret ;
}

int VideoThread::prepareSWSContextAndRGBAFrame(int videoStreamWidth,
                                   int videoStreamHeight,
                                   enum AVPixelFormat videoStreamFormat){

    int frameNumBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                 videoStreamWidth,
                                                 videoStreamHeight,
                                                 1);
    //should av_free
    mpForWindowFrameBuffer = (uint8_t*)av_malloc(frameNumBytes * sizeof(uint8_t));

    //alloc frame and fill it
    //should av_frame_free
    mpNativeWindowRGBAFrame = av_frame_alloc();
    int ret = av_image_fill_arrays(mpNativeWindowRGBAFrame->data,
                                   mpNativeWindowRGBAFrame->linesize,
                                   mpForWindowFrameBuffer,
                                   AV_PIX_FMT_RGBA,
                                   videoStreamWidth,
                                   videoStreamHeight,
                                   1);
    if(0 > ret){
        LOGE("VideoThread prepareSWSContextAndRGBAFrame : av_image_fill_arrays fail->%d", ret);
        return -1;
    }

    mpSwsContext = sws_getContext(videoStreamWidth,
                                videoStreamHeight,
                                videoStreamFormat,
                                videoStreamWidth,
                                videoStreamHeight,
                                AV_PIX_FMT_RGBA,
                                SWS_BICUBIC/*算法种类*/,
                                nullptr,
                                nullptr,
                                nullptr);
    if(nullptr == mpSwsContext){
        LOGE("VideoThread prepareSWSContextAndRGBAFrame : sws_getContext fail!");
        return -1;
    }

    mPrepared = true;
    return 0;
}

int VideoThread::stop(){
    mAbort = 1;
    Thread::stop();
    return 0;
}

void VideoThread::run(){
    LOGD("VideoThread run : begin!");
    if(!mPrepared){
        LOGE("VideoThread run : not prepared!");
        return;
    }

    int nullFrameGetCount = 0;
    while(1 != mAbort){

        AVFrame* pFrame = mpAVFrameQueue->pop(10);
        if(!pFrame){
            LOGW("VideoThread run : do not get video frame! frame queue size->%d",
                 mpAVFrameQueue->size());
            ++nullFrameGetCount;
            //if(5 < nullFrameGetCount) break; //more than 5 times then end!
            continue;
        }
        nullFrameGetCount = 0;
        LOGD("VideoThread run : get a video frame!");

        refreshForNativeWindow(pFrame);
    }

    LOGD("VideoThread run : finish!");
}

void VideoThread::refreshForNativeWindow(AVFrame* yuvFrame){
    //convert
    sws_scale(mpSwsContext,
              yuvFrame->data,
              yuvFrame->linesize,
              0,
              mVideoStreamHeight,
              mpNativeWindowRGBAFrame->data,
              mpNativeWindowRGBAFrame->linesize
            );

    ANativeWindow_lock(mpNativeWindow, &mNativeWindowBuffer, nullptr);
    uint8_t* windowBuffer_uint8 = (uint8_t*)mNativeWindowBuffer.bits;
    //window buffer line size(RGBA:4bytes)
    int windowLineSize = mNativeWindowBuffer.stride * 4;
    for(int k = 0; k < mVideoStreamHeight; ++k){
        uint8_t* dst = windowBuffer_uint8 + windowLineSize * k;
        //line size : byte count
        uint8_t* src = mpForWindowFrameBuffer + k * mpNativeWindowRGBAFrame->linesize[0];
        int copyLenght = mpNativeWindowRGBAFrame->linesize[0];
        memcpy(dst, src, copyLenght);
    }

    //frame's time base not correct (AVStream's is correct)
    /**
     * FIX ME
     * the code not perfect!
     */
    uint64_t pts_time = yuvFrame->pts * av_q2d(mTimeBase) * 1000;
    auto now = std::chrono::system_clock::now();
    if(0 == yuvFrame->pts){
        refreshStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    uint64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    uint64_t time_diff = pts_time - (now_ms - refreshStartTime);
    LOGD("VideoThread refreshForNativeWindow : yuvFrame pts time->%d refreshStartTime->%d now_ms->%d time_diff->%d",
         pts_time, refreshStartTime, now_ms , time_diff);

    if(0 < time_diff)
        std::this_thread::sleep_for(std::chrono::milliseconds(time_diff));

    //post to refresh!
    ANativeWindow_unlockAndPost(mpNativeWindow);

    av_frame_free(&yuvFrame);
}