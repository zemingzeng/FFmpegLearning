#ifndef  AVPACKETQUEUE_H
#define  AVPACKETQUEUE_H

#include "safe_queue/SafeQueue.h"

extern "C"{
#include "libavcodec/packet.h"
};

using namespace mingzz;

class AVPacketQueue{

public:
    AVPacketQueue();
    ~AVPacketQueue();

    void abort();

    int push(AVPacket* t);

    AVPacket* pop(int timeOut = 0);

    AVPacket* front();

    int size();

private:
    SafeQueue<AVPacket*> mQueue;

};


#endif
