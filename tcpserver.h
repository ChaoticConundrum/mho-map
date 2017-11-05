#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "zthread.h"
#include "zstreamsocket.h"
#include "zconnection.h"
#include "zcondition.h"
#include "zmutex.h"
#include "zqueue.h"
#include "zpointer.h"
#include "zworkqueue.h"
#include "zmap.h"

#include "connection.h"

using namespace LibChaos;

class TCPServer {
public:
    struct ThreadData {
        ZPointer<ZThread> thread;
        ZCondition cond;
        ZMutex mutex;
    };

public:
    TCPServer(zu16 threadcount);
    ~TCPServer();

    void run();

private:
    void initListen();
    void acceptConnection();
    void readConnection(zsocktype fd);
    void writeConnection(zsocktype fd);

    void addWatchEvent(int fd, unsigned flags);
    void modWatchEvent(int fd, unsigned flags);
    void delWatchEvent(int fd);

    static void *threadFunc(ZThread::ZThreadArg zarg);

    void createThreads();
    void stopThreads();
    void joinThreads();

private:
    ZStreamSocket socket;
    ZAddress bindaddr;

    int epfd;

    ZMap<zsocktype, ZPointer<Connection>> connections;
    ZMap<int, zsocktype> out_events;

    zu16 threadcount;
    ZArray<ZPointer<ThreadData>> threaddata;

public:
    ZWorkQueue<ZPointer<Connection>> queue;
};

#endif // ARCHONSERVER_H
