/**
 * @file DecodeThread.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-12-07
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "ffmpeg_thread/DecodeThread.h"
#include "log/Log.h"

extern "C"{
#include "libavutil/error.h"
}

using namespace mingzz;

#define DECODETHREAD_DEBUG_ON true

#define IF_DECODETHREAD_DEBUG_ON if( \
                 DECODETHREAD_DEBUG_ON \
                 )

DecodeThread::DecodeThread(AVPacketQueue*pktQueue,AVFrameQueue*frameQueue) :
                                          mpAVFQueue(frameQueue),
                                          mpAVPQueue(pktQueue),
                                          mpAVCodecContext(nullptr)

{
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread(*,*)!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));
}

DecodeThread::~DecodeThread(){
    IF_DECODETHREAD_DEBUG_ON LOGD("~DecodeThread()!");


}

int DecodeThread::stop(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

    return 0;
}

void DecodeThread::run(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

}

int DecodeThread::init(AVCodecParameters* avCodecParams){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop!");

    if(!avCodecParams){
        LOGE("DecodeThread init : avCodecParams is nullptr");
        return -1;
    }

    //should be freed with avcodec_free_context()
    mpAVCodecContext = avcodec_alloc_context3(nullptr);
    if(!mpAVCodecContext){
        LOGE("DecodeThread init : avcodec_alloc_context3 return nullptr");
        return -1;
    }

    //Fill the codec context field from params
    int ret = avcodec_parameters_to_context(mpAVCodecContext,avCodecParams);
    if(0 > ret){
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo));
        LOGE("DecodeThread init : avcodec_parameters_to_context error->%s",mAVErrorInfo);
        return -1;
    }

    //Find a registered decoder with a matching codec ID
    const AVCodec* pAVCodec = avcodec_find_decoder(mpAVCodecContext->codec_id);
    if(pAVCodec){
        const char* avCodecName = avcodec_get_name(mpAVCodecContext->codec_id);
        IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread int : find decoder name->%s",avCodecName);
    } else {
        LOGE("DecodeThread int : avcodec_find_decoder error codec id->%d!",mpAVCodecContext->codec_id);
        return -1;
    }

    //Initialize the AVCodecContext to use the given AVCodec.
    ret = avcodec_open2(mpAVCodecContext, pAVCodec, nullptr);

    if(0 > ret){
        av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo));
        LOGE("DecodeThread init : avcodec_parameters_to_context error->%s",mAVErrorInfo);
        return -1;
    }

    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread int : finish");
    return 0;
}