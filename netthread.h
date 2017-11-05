#ifndef NETTHREAD_H
#define NETTHREAD_H

#include "zthread.h"

using namespace LibChaos;

class NetThread : public ZThread::ZThreadContainer {
public:
    NetThread();

    // ZThread interface
private:
    void *run(void *arg);
};

#endif // ARCHONTHREAD_H
