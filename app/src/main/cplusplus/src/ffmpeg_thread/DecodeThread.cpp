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
                                          mpAVCodecContext(nullptr),
                                          mAbort(0)

{
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread(*,*)!");

    memset(mAVErrorInfo, 0, sizeof(mAVErrorInfo));
}

DecodeThread::~DecodeThread(){
    IF_DECODETHREAD_DEBUG_ON LOGD("~DecodeThread()!");

    stop();

    if(mpAVCodecContext){
        avcodec_free_context(&mpAVCodecContext);
    }
}

int DecodeThread::stop(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread stop : mAbort->%d addr->%p", mAbort, this);

    mAbort = 1;

    Thread::stop();

    return 0;
}

void DecodeThread::run(){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread run!");
    //reference
    //https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c

    if(!mpAVPQueue || !mpAVCodecContext || !mpAVFQueue){
        LOGE("DecodeThread run : mpAVPQueue or mpAVCodecContext or mpAVFQueue is nullptr");
        return ;
    }

    AVFrame* pFrame =av_frame_alloc();

    int nullPacketGetCount = 0;
    while(1 != mAbort){

        //wait for frame consumed!
        if(5 < mpAVFQueue->size()){
            IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread run : wait for frame consumed! addr->%p", this);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

         AVPacket* pPacket = mpAVPQueue->pop(10);
         if(!pPacket){
             ++nullPacketGetCount;
             LOGW("DecodeThread run : pPacket(is nullptr) does not get! packet queue size->%d",mpAVPQueue->size());
             //if(5 < nullPacketGetCount) break; //get null packet more than 5 times then stop!
             continue;
         }
         nullPacketGetCount = 0;

        //Ownership of the packet remains with the caller
        //should free by yourself
         int ret = avcodec_send_packet(mpAVCodecContext, pPacket);
         av_packet_free(&pPacket);
         if(ret < 0){
             av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo));
             LOGE("DecodeThread run : avcodec_send_packet error->%s",mAVErrorInfo);
             break;
         }

         //读取解码好的frame
         while(true){
             ret = avcodec_receive_frame(mpAVCodecContext, pFrame);

             if(0 == ret){
                 IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread run : get a frame, frame size->%d",mpAVFQueue->size());
                 mpAVFQueue->push(pFrame);
                 continue;
             } else {
                 av_strerror(ret, mAVErrorInfo, sizeof(mAVErrorInfo));
                 if(AVERROR(EAGAIN) == ret){
                     //output is not available in this state - user must try to send new input
                     LOGW("DecodeThread run : avcodec_receive_frame error->%s",mAVErrorInfo);
                     break;
                 } else {
                     //may error happen ! stop the thread!
                     mAbort = 1;
                     LOGE("DecodeThread run : avcodec_receive_frame error->%s",mAVErrorInfo);
                     break;
                 }
             }
         }
    }

    //we alloc the frame and we should free it
    av_frame_free(&pFrame);

    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread run : finish!");

}

int DecodeThread::init(AVCodecParameters* avCodecParams){
    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread init!");

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

    //LOGD("DecodeThread init : sampleRate->%d,sampleFormat->%d,nb_channels->%d",
           //sampleRate, sampleFormat, avChannelLayout.nb_channels);

    IF_DECODETHREAD_DEBUG_ON LOGD("DecodeThread int : finish");
    return 0;
}

//only for video
int DecodeThread::getVideoHeightFromCodecContext(){
    if(!mpAVCodecContext){
        LOGE("DecodeThread getVideoHeightFromCodecContext : mpAVCodecContext = null");
        return -1;
    }
    //audio try to get then return invalid
    if(AVMEDIA_TYPE_AUDIO == mpAVCodecContext->codec_type){
        return -1;
    }
    return mpAVCodecContext->height;
}

int DecodeThread::getVideoWidthFromCodecContext(){
    if(!mpAVCodecContext){
        LOGE("DecodeThread getVideoWidthFromCodecContext : mpAVCodecContext = null");
        return -1;
    }
    //audio try to get then return invalid
    if(AVMEDIA_TYPE_AUDIO == mpAVCodecContext->codec_type){
        return -1;
    }
    return mpAVCodecContext->width;
}

enum AVPixelFormat DecodeThread::getVideoPixFormatFromCodecContext(){
    if(!mpAVCodecContext){
        LOGE("DecodeThread getVideoPixFormatFromCodecContext : mpAVCodecContext = null");
        return AV_PIX_FMT_NONE;
    }
    if(AVMEDIA_TYPE_AUDIO == mpAVCodecContext->codec_type){
        return AV_PIX_FMT_NONE;
    }
    return mpAVCodecContext->pix_fmt;
}

//only for audio
int DecodeThread::getAudioSampleRateFormCodecContext(){
    return mpAVCodecContext->sample_rate;
}

enum AVSampleFormat DecodeThread::getAudioSampleFormatFormCodecContext(){
    return mpAVCodecContext->sample_fmt;
}

AVChannelLayout DecodeThread::getAudioChannelLayoutFormCodecContext(){
    return mpAVCodecContext->ch_layout;
}
