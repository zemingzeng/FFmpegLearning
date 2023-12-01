#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include "log/Log.h"

namespace mingzz {

    class Thread {
    public:
        Thread();

        virtual ~Thread();

        int start();

        void join();

        void detach();

        int stop();

    private:
        virtual void run() = 0;

    private:
        std::thread *mpThread;
    };
}

#endif
