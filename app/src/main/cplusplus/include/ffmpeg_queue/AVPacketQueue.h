/**
 * @file AVPacketQueue.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef  AVPACKETQUEUE_H
#define  AVPACKETQUEUE_H

#include "safe_queue/SafeQueue.h"

extern "C"{
#include "libavcodec/packet.h"
};

namespace mingzz {

class AVPacketQueue{

public:

    AVPacketQueue();

    ~AVPacketQueue();

    void abort();

    int size();

    int push(AVPacket* t);

    AVPacket* pop(const int timeOut = 10);

    AVPacket* front();

private:

    SafeQueue<AVPacket*> mQueue;

    void release();

};

}

#endif
