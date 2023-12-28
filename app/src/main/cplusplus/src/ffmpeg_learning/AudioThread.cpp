#include "ffmpeg_learning/AudioThread.h"

//ffmpeg
#define SWR_OUT_DEFAULT_SAMPLE_RATE 44100
#define SWR_OUT_DEFAULT_CHANNELS 2
#define SWR_OUT_DEFAULT_SAMPLE_FORMAT_BYTES 2 //默认16位，2bytes
#define SWR_OUT_DEFAULT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define SWR_OUT_DEFAULT_CHANNEL_LAYOUT AV_CHANNEL_LAYOUT_STEREO
#define SWR_OUT_BUFFER_SIZE ((unsigned long)SWR_OUT_DEFAULT_SAMPLE_RATE * SWR_OUT_DEFAULT_CHANNELS * SWR_OUT_DEFAULT_SAMPLE_FORMAT_BYTES)
//opensl
#define OPENSL_DEFAULT_CHANNELS SWR_OUT_DEFAULT_CHANNELS
#define OPENSL_DEFAULT_SAMPLE_RATE SWR_OUT_DEFAULT_SAMPLE_RATE
#define OPENSL_DEFAULT_SAMPLE_FORMAT SL_PCMSAMPLEFORMAT_FIXED_16 //according to SWR
#define OPENSL_DEFAULT_SAMPLE_FORMAT_BYTES SWR_OUT_DEFAULT_SAMPLE_FORMAT_BYTES

static void slBufferQueueCallback_(SLAndroidSimpleBufferQueueItf bufferQueue, void *pContext);
static uint8_t *mOutAudioBuffer = nullptr;

AudioThread::~AudioThread(){

    if(mpSwrContext)
        swr_free(&mpSwrContext);
    if(mOutAudioBuffer)
        av_free(mOutAudioBuffer);

    //release opensl
    if(mpSLAudioPlayer){
        (*mpSLAudioPlayer)->Destroy(mpSLAudioPlayer);
    }
    if(mpSLMixerObject){
        (*mpSLMixerObject)->Destroy(mpSLMixerObject);
    }
    if(mpSLEngineObject){
        (*mpSLEngineObject)->Destroy(mpSLEngineObject);
    }

}

AudioThread::AudioThread(AVFrameQueue* avframeQueue,
                         int sampleRate,
                         enum AVSampleFormat sampleFormat,
                         AVChannelLayout avChannelLayout) :
                              mpAVFrameQueue{avframeQueue},
                              mpSwrContext{nullptr},
                              mSampleRate{sampleRate},
                              mSampleFormat{sampleFormat},
                              mAVChannelLayout{avChannelLayout},
                              mAbort(0),
                              mPrepared(false),
                              mpSLEngineObject{nullptr},
                              mpSLMixerObject{nullptr},
                              mpSLAudioPlayer{nullptr}
{
       LOGD("AudioThread constructor : sampleRate->%d,sampleFormat->%d,nb_channels->%d",
            sampleRate, sampleFormat, avChannelLayout.nb_channels);
}

int AudioThread::init(){
    prepareSwrContext();
    prepareOpenSL();
    mPrepared = (nullptr == mpAVFrameQueue || nullptr == mpSwrContext ) ? false : true;
    if(!mPrepared){
        return -1;
    }
    return 0;
}

int AudioThread::stop(){
    return 0;
}

void AudioThread::prepareSwrContext(){
    //should free
    mpSwrContext = swr_alloc();
    //AVSampleFormat outSampleFormat = SWR_OUT_DEFAULT_SAMPLE_FORMAT;
    //int outSampleRate = SWR_OUT_DEFAULT_SAMPLE_RATE;
    AVChannelLayout outChannelLayout = SWR_OUT_DEFAULT_CHANNEL_LAYOUT;
    swr_alloc_set_opts2(&mpSwrContext,
                        &outChannelLayout,
                        SWR_OUT_DEFAULT_SAMPLE_FORMAT,
                        SWR_OUT_DEFAULT_SAMPLE_RATE,
                        &mAVChannelLayout,
                        mSampleFormat,
                        mSampleRate,
                        0,
                        nullptr);
    swr_init(mpSwrContext);
    //1s的pcm数据字节数 44100*2:双通道44.1khz,16位(2字节)(想要转化的成的音频数据格式)
    mOutAudioBuffer = (uint8_t *) av_malloc(SWR_OUT_BUFFER_SIZE);
}

void AudioThread::prepareOpenSL(){

    /**
     * 使用opensl es的步骤
     * 1.创建引擎(创建引擎对象，初始化，获取引擎接口)
     * 2.设置混音器
     * 3.创建播放器
     * 4.开始播放
     * 5。停止播放
     */
    SLresult result = SL_RESULT_UNKNOWN_ERROR;

    //SLObjectItf engineObject = nullptr;
    result = slCreateEngine(&mpSLEngineObject, 0,
                            nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Engine Create fail");
        return;
    }
    result = (*mpSLEngineObject)->Realize(mpSLEngineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Engine Realize fail");
        return;
    }
    SLEngineItf engineInterface = nullptr;
    result = (*mpSLEngineObject)->GetInterface(mpSLEngineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Engine GetInterface fail");
        return;
    }
    LOGD("AudioThread prepareOpenSL : SL Engine initialize finish!");

    //SLObjectItf mixerObject = nullptr;
    SLInterfaceID mixerInterfaceIds[1] = {SL_IID_ENVIRONMENTALREVERB};
    SLboolean mixerInterfaceRequired[]{SL_BOOLEAN_FALSE};
    result = (*engineInterface)->CreateOutputMix(engineInterface,
                                                 &mpSLMixerObject,
                                                 1, mixerInterfaceIds,
                                                 mixerInterfaceRequired);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : CreateOutputMix fail");
        return;
    }
    result = (*mpSLMixerObject)->Realize(mpSLMixerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Mixer Realize fail");
        return;
    }
    SLEnvironmentalReverbItf mixer = nullptr;
    result = (*mpSLMixerObject)->GetInterface(mpSLMixerObject, SL_IID_ENVIRONMENTALREVERB, &mixer);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Mixer GetInterface fail");
        return;
    }
    LOGD("AudioThread prepareOpenSL : Mixer initialize finish!");

    //player播放参数设置
    SLDataFormat_PCM formatPcm{
            .formatType=SL_DATAFORMAT_PCM,
            .numChannels=OPENSL_DEFAULT_CHANNELS,
            .samplesPerSec=createOpenSLSampleRate(OPENSL_DEFAULT_SAMPLE_RATE),
            //采样位数16
            .bitsPerSample=OPENSL_DEFAULT_SAMPLE_FORMAT,
            //和采样位数一致即可
            .containerSize=OPENSL_DEFAULT_SAMPLE_FORMAT,
            .endianness=SL_BYTEORDER_LITTLEENDIAN
    };
    //SLObjectItf player = nullptr;
    SLDataLocator_AndroidSimpleBufferQueue androidSimpleBufferQueue{
            .locatorType=SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            //一个采样点2字节，因为采样率为16
            .numBuffers=OPENSL_DEFAULT_SAMPLE_FORMAT_BYTES
    };
    SLDataSource dataSource{
            .pLocator=&androidSimpleBufferQueue,
            .pFormat = &formatPcm
    };
    SLDataLocator_OutputMix outputMix{
            .locatorType=SL_DATALOCATOR_OUTPUTMIX,
            .outputMix=mpSLMixerObject
    };
    SLDataSink dataSink{
            .pLocator=&outputMix,
            .pFormat=nullptr
    };
    SLInterfaceID playerInterfaceIds[]{SL_IID_MUTESOLO, SL_IID_VOLUME, SL_IID_BUFFERQUEUE};
    SLboolean playerInterfaceRequired[]{SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &mpSLAudioPlayer,
                                                   &dataSource, &dataSink,
                                                   sizeof(playerInterfaceRequired) /
                                                   sizeof(SL_BOOLEAN_TRUE),
                                                   playerInterfaceIds,
                                                   playerInterfaceRequired);
    LOGD("AudioThread prepareOpenSL : sizeof(playerInterfaceRequired) / sizeof(SL_BOOLEAN_TRUE) : %ld",
         sizeof(playerInterfaceRequired) / sizeof(SL_BOOLEAN_TRUE));
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : CreateAudioPlayer fail");
        return;
    }
    result = (*mpSLAudioPlayer)->Realize(mpSLAudioPlayer, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Player Realize fail");
        return;
    }
    SLPlayItf playerInterface = nullptr;
    result = (*mpSLAudioPlayer)->GetInterface(mpSLAudioPlayer, SL_IID_PLAY, &playerInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : Player GetInterface fail");
        return;
    }
    LOGD("AudioThread prepareOpenSL : Player initialize finish!");
    LOGD("AudioThread prepareOpenSL : all initialize finish!");

    SLMuteSoloItf muteInterface = nullptr;
    SLVolumeItf volumeInterface = nullptr;
    SLAndroidSimpleBufferQueueItf bufferQueueInterface = nullptr;
    result = (*mpSLAudioPlayer)->GetInterface(mpSLAudioPlayer, SL_IID_MUTESOLO, &muteInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : player GetInterface SL_IID_MUTESOLO fail");
        return;
    }
    result = (*mpSLAudioPlayer)->GetInterface(mpSLAudioPlayer, SL_IID_VOLUME, &volumeInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : player GetInterface SL_IID_VOLUME fail");
        return;
    }
    //callback被主动调用，用来获取需要播放的数据
    result = (*mpSLAudioPlayer)->GetInterface(mpSLAudioPlayer, SL_IID_BUFFERQUEUE, &bufferQueueInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : player GetInterface SL_IID_BUFFERQUEUE fail");
        return;
    }
    result = (*bufferQueueInterface)->RegisterCallback(bufferQueueInterface, slBufferQueueCallback_,
                                                       this);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : bufferQueueInterface RegisterCallback fail");
        return;
    }
    //先设置为播放状态
    result = (*playerInterface)->SetPlayState(playerInterface, SL_PLAYSTATE_PLAYING);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : playerInterface SetPlayState fail");
        return;
    }
    //启动buffer queue队列回调
    result = (*bufferQueueInterface)->Enqueue(bufferQueueInterface, mOutAudioBuffer, 1);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread prepareOpenSL : bufferQueueInterface Enqueue fail");
        return;
    }

}

uint32_t AudioThread::createOpenSLSampleRate(int rate) {
    uint32_t sampleRate = 0;
    switch (rate) {
        case 44100:
            sampleRate = SL_SAMPLINGRATE_44_1;
            break;
        case 12000:
            sampleRate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            sampleRate = SL_SAMPLINGRATE_16;
            break;
        case 88200:
            sampleRate = SL_SAMPLINGRATE_88_2;
            break;
        case 24000:
            sampleRate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            sampleRate = SL_SAMPLINGRATE_32;
            break;
        case 64000:
            sampleRate = SL_SAMPLINGRATE_64;
            break;
        default:
            break;
    }
    LOGD("AudioThread createOpenSLSampleRate : rate->%d", sampleRate);
    return sampleRate;
}

void AudioThread::run(){
}

int AudioThread::prepareFrameBuffer(){

    if(!mPrepared){
        LOGE("AudioThread prepareFrameBuffer : not prepared!");
        return 0;
    }

    int nullFrameGetCount = 0;
    while(1 != mAbort){

        AVFrame* pFrame = mpAVFrameQueue->pop(10);
        if(!pFrame){
            LOGW("AudioThread run : do not get audio frame! frame queue size->%d",
                 mpAVFrameQueue->size());
            ++nullFrameGetCount;
            //if(5 < nullFrameGetCount) break; //more than 5 times then end!
            continue;
        }
        nullFrameGetCount = 0;
        LOGD("AudioThread run : get a audio frame!");

        int oneChannelSamples = swr_convert(mpSwrContext, &mOutAudioBuffer,
                                            pFrame->nb_samples,
                                            (const uint8_t **) pFrame->data,
                                            pFrame->nb_samples);
        if(0 >= oneChannelSamples){
            LOGW("AudioThread run : oneChannelSamples is invalid!");
            av_frame_free(&pFrame);
            continue;
        }
        LOGD("AudioThread run : convert finish oneChannelSamples->%d", oneChannelSamples);

        int actualFillSize = oneChannelSamples * 2 * 2;
        av_frame_free(&pFrame);

        return actualFillSize;
    }

    return 0;
}



//底层调用就证明需要数据，把解码好的pcm数据传给底层
void slBufferQueueCallback_(SLAndroidSimpleBufferQueueItf bufferQueue, void *pContext) {
    LOGD("AudioThread : slBufferQueueCallback.......");
    SLuint32 result = SL_RESULT_UNKNOWN_ERROR;
    AudioThread* pAudioThread = (AudioThread*)pContext;
    int bufferSize = pAudioThread->prepareFrameBuffer();
    result = (*bufferQueue)->Enqueue(bufferQueue, mOutAudioBuffer, (SLuint32) bufferSize);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioThread : slBufferQueueCallback bufferQueueInterface Enqueue fail");
        return;
    }
}