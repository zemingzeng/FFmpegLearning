/**
 * @file Thread.cpp
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "thread/Thread.h"
#include "log/Log.h"

using namespace mingzz;

#define THREAD_DEBUG_ON true

#define IF_THREAD_DEBUG_ON if( \
                 THREAD_DEBUG_ON \
                 )

#define THREAD_DEBUG_ON true

Thread::Thread() : mpThread(nullptr) {
    IF_THREAD_DEBUG_ON LOGD("Thread::Thread()!");

}

Thread::~Thread() {
    IF_THREAD_DEBUG_ON LOGD("Thread::~Thread()!");

    stop();
}

int Thread::start() {
    IF_THREAD_DEBUG_ON  LOGD("Thread::start()!");

    mpThread = new std::thread(&Thread::run, this); // new完，thread就自动启动了
                                                    // 子类调用到这时会指定对应的虚表里面的函数指针
    if (!mpThread) {
        LOGE("new std::thread error!");
        return -1;
    }
    return 0;
}

int Thread::stop() {
    IF_THREAD_DEBUG_ON LOGD("Thread::stop()!");

    if (mpThread) {
        IF_THREAD_DEBUG_ON LOGD("Thread before delete t!");
        // 等待任务完成然后才delete,
        // 会阻塞此线程直到mpThread任务处理完成然后返回
        join();
        delete mpThread;
        if (THREAD_DEBUG_ON)
            LOGD("Thread after delete t!");
        mpThread = nullptr;
    }

    return 0;
}

void Thread::join() {
    IF_THREAD_DEBUG_ON LOGD("Thread::join()!");
    if (mpThread) {
        if (mpThread->joinable())
            mpThread->join();
    }
}

void Thread::detach() {
    IF_THREAD_DEBUG_ON LOGD("Thread::detach()!");
    if (mpThread) {
        if (mpThread->joinable())
            mpThread->detach();
    }
}