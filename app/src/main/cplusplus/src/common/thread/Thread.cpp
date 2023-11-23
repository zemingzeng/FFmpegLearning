#include "thread/Thread.h"
#include "log/Log.h"

using namespace mingzz;

#define THREAD_DEBUG_ON true

Thread::Thread() : mpThread(nullptr) {
    if (THREAD_DEBUG_ON)
        LOGD("Thread::Thread()!");
}

Thread::~Thread() {
}

int Thread::start() {
    if (THREAD_DEBUG_ON)
        LOGD("Thread::start()!");
    mpThread = new std::thread(&Thread::run, this); // new完，thread就自动启动了
    if (!mpThread) {
        LOGE("new std::thread error!");
        return -1;
    }
    return 0;
}

int Thread::stop() {
    if (THREAD_DEBUG_ON)
        LOGD("Thread::stop()!");
    if (mpThread) {
        if (THREAD_DEBUG_ON)
            LOGD("Thread before delete t!");
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
    if (THREAD_DEBUG_ON)
        LOGD("Thread::join()!");
    if (mpThread) {
        if (mpThread->joinable())
            mpThread->join();
    }
}

void Thread::detach() {
    if (THREAD_DEBUG_ON)
        LOGD("Thread::detach()!");
    if (mpThread) {
        if (mpThread->joinable())
            mpThread->detach();
    }
}