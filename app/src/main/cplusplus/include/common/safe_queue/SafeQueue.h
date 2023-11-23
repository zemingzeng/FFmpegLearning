/**
 * @file SafeQueue.h
 * @author your name (zemingzeng@126.com)
 * @brief
 * @version 0.1
 * @date 2023-11-17
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

namespace mingzz {

    template<typename T>
    class SafeQueue {
    public:
        SafeQueue();

        ~SafeQueue();

        void abort();

        int push(T t);

        int pop(T &t, int timeOut = 0);

        int front(T &t);

        int size();

    private:
        int mAbort;
        std::queue <T> mQueue;
        std::mutex mMutex;
        std::condition_variable mCondition;
    };

#include "safe_queue/SafeQueue.template"

}
#endif
