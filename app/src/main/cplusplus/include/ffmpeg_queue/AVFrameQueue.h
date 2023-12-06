/**
 * @file AVFrameQueue.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-12-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef AVFRAMEQUEUE_H
#define AVFRAMEQUEUE_H

#include "safe_queue/SafeQueue.h"

extern "C" {
#include "libavutil/frame.h"
};

class AVFrameQueue {

public:

    AVFrameQueue();

    ~AVFrameQueue();

    void abort();

    int size();

    int push(AVFrame*);

    AVFrame* pop(const int timeOut = 10);

    AVFrame* front();

private:

    SafeQueue<AVFrame*> mQueue;

};

#endif