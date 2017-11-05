#include "tcpserver.h"
#include "netthread.h"

#include "zlog.h"
#include "zerror.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdlib>
#include <netdb.h>

#define MAX_EVENTS      256
#define WAIT_TIMEOUT    10000

#define READ_WINDOW     1024
#define WRITE_WINDOW    1024

static bool runflag = true;

void stopHandler(ZError::zerror_signal sig){
    runflag = false;
}

TCPServer::TCPServer(zu16 tcount) : bindaddr(8181), threadcount(tcount){

}

TCPServer::~TCPServer(){
    socket.close();
}

void TCPServer::initListen(){
    ZError::registerInterruptHandler(stopHandler);
    ZError::registerSignalHandler(ZError::TERMINATE, stopHandler);

    LOG("Bind to " << bindaddr.debugStr());
    socket.setAllowRebind(true);
    if(!socket.listen(bindaddr)){
        socket.close();
        throw ZException("Socket Listen Fail");
    }
    if(!socket.setBlocking(false)){
        socket.close();
        throw ZException("Socket Option Fail");
    }
    LOG("Listening...");
}

void TCPServer::acceptConnection(){
    ZPointer<ZConnection> conn;
    ZSocket::socketerror err;
    // Accept connections
    while(true){
        err = socket.accept(conn);
        // On error, handle outside loop
        if(err != ZSocket::OK)
            break;

        zsocktype sock = conn->getSocket();

        LOG(conn->peer().debugStr() << " New Connection");
        // Make connection non blocking
        if(!conn->setBlocking(false)){
            ELOG("Failed set nonblocking");
        }

        // Setup event
        addWatchEvent(sock, EPOLLIN | EPOLLET);

        // Add eventfd
        int efd = eventfd(0, EFD_NONBLOCK);
        addWatchEvent(efd, EPOLLIN | EPOLLET);

        // Setup connection data
        ZPointer<Connection> cdata = new Connection;
        cdata->connection = conn;
        cdata->out_eventfd = efd;
        cdata->final = false;

        connections[sock] = cdata;
        out_events[efd] = sock;
    }

    // Done accepting
    if(err != ZSocket::AGAIN){
        ELOG("Accept Error");
    }
}

void TCPServer::readConnection(zsocktype fd){
    ZPointer<Connection> conn = connections[fd];
    ZString address = conn->connection->peer().debugStr();
    ZBinary buffer;
    ZSocket::socketerror err;

    conn->lock.lock();

    // Read all available data
    while(true){
        buffer.resize(READ_WINDOW);
        err = conn->connection->read(buffer);
        if(err != ZSocket::OK)
            break;
        conn->in_buffer.concat(buffer);
    }

    zu64 len = conn->in_buffer.size();

    // Done reading
    if(err == ZSocket::AGAIN){
        // Read all data, connection open
        LOG(address << " Read " << len);

        // Queue work
        conn->status = Connection::READ;
        queue.addWork(conn);

    } else {
        if(err == ZSocket::DONE){
            LOG(address << " Closed");
        } else {
            ELOG(address << " Read Error");
        }
        conn->status = Connection::CLOSE;

        // Unregister event
        delWatchEvent(fd);
        // Remove connection
        connections.remove(fd);
    }

    conn->lock.unlock();
}

void TCPServer::writeConnection(zsocktype fd){
    ZPointer<Connection> conn = connections[fd];
    ZString address = conn->connection->peer().debugStr();
    ZBinary buffer;
    ZSocket::socketerror err;
    zu64 len = 0;

    conn->lock.lock();
    zu64 outsize = conn->out_buffer.size();
    bool final = conn->final;

    // Write all data possible
    while(outsize){
        // Get write data
        buffer.write(conn->out_buffer.raw(), MIN(outsize, WRITE_WINDOW));

        err = conn->connection->write(buffer);
        if(err != ZSocket::OK)
            break;

        // Write ok, remove written data
        conn->out_buffer = conn->out_buffer.getSub(buffer.size(), outsize - buffer.size());
        len += buffer.size();
        buffer.clear();
        outsize = conn->out_buffer.size();
    }

    // Done writing
    if(err == ZSocket::AGAIN){
        // Continue writing
        LOG(address << " Write " << len);

    } else if(!outsize){
        LOG(address << " Write " << len);

        // Stop waiting for output
        modWatchEvent(fd, EPOLLIN | EPOLLET);

        if(final){
            // Final data
            LOG(address << " Done");
            conn->status = Connection::DONE;
        } else {
            // Otherwize, queue work again
            conn->status = Connection::WRITE;
            queue.addWork(conn);
        }

    } else {
        if(err == ZSocket::DONE){
            LOG(address << " Closed");
        } else {
            ELOG(address << " Write Error: " << ZSocket::errorStr(err));
        }
        conn->status = Connection::CLOSE;

        // Unregister event
        delWatchEvent(fd);
        // Remove connection
        connections.remove(fd);
    }

    conn->lock.unlock();
}

void TCPServer::run(){
    createThreads();
    initListen();

    zsocktype listener = socket.getSocket();

    epfd = epoll_create1(0);
    if(epfd == -1){
        ELOG("epoll create failed");
        return;
    }

    // register the listener socket
    epoll_event event;
    event.data.fd = listener;
    event.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &event);
    if(ret == -1){
        ELOG("epoll ctl failed");
        return;
    }

    epoll_event events[MAX_EVENTS];
    ZAllocator<epoll_event>::zero(events, MAX_EVENTS);

    // main loop
    while(runflag){
        ret = epoll_wait(epfd, events, MAX_EVENTS, WAIT_TIMEOUT);
        if(ret < 0){
            //ELOG("epoll wait failed " << ZError::getSystemError());
            continue;
        }

        for(int i = 0; i < ret; ++i){
            zsocktype fd = events[i].data.fd;
            if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP){
                // Error or HUP on socket
                ELOG("epoll Error - socket " << fd);

                // Unregister event
                delWatchEvent(fd);
                // Remove connection
                connections.remove(fd);

            } else if(events[i].events & EPOLLIN){
                // Input ready
                if(fd == listener){
                    // Activity on the listener socket
                    acceptConnection();

                } else if(out_events.contains(fd)){
                    // Register socket for output
//                    LOG("Mark output");
                    modWatchEvent(out_events[fd], EPOLLIN | EPOLLOUT | EPOLLET);

                } else if(connections.contains(fd)){
                    // Connection socket ready for read
                    readConnection(fd);

                } else {
                    ELOG("no fd");
                }

            } else if(events[i].events & EPOLLOUT){
                // Output ready
                if(connections.contains(fd)){
                    // Connection socket ready for write
                    writeConnection(fd);

                } else {
                    ELOG("no fd");
                }

            } else {
                // No event on socket
                ELOG("No Event - socket " << fd);

                // Unregister event
                delWatchEvent(fd);
                // Remove connection
                connections.remove(fd);
            }
        }
    }

    LOG("STOP");

    joinThreads();
}

void TCPServer::addWatchEvent(int fd, unsigned flags){
    epoll_event event;
    event.data.fd = fd;
    event.events = flags;
    // Register event
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    if(ret == -1){
        ELOG("epoll ctl add failed");
    }
}

void TCPServer::modWatchEvent(int fd, unsigned flags){
    epoll_event event;
    event.data.fd = fd;
    event.events = flags;
    // Register event
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
    if(ret == -1){
        ELOG("epoll ctl mod failed");
    }
}

void TCPServer::delWatchEvent(int fd){
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    if(ret == -1){
        ELOG("epoll ctl del failed");
    }
}

void TCPServer::createThreads(){
    for(zu64 i = 0; i < threadcount; ++i){
        ZPointer<ThreadData> data = new ThreadData;
        // Create thread
        data->thread = new ZThread(new NetThread);
        // Run thread
        data->thread->exec(this);

        threaddata.push(data);
    }
}

void TCPServer::stopThreads(){
    queue.addWork(nullptr);
}

void TCPServer::joinThreads(){
    stopThreads();
    for(zu64 i = 0; i < threaddata.size(); ++i){
        threaddata[i]->thread->join();
        LOG("Joined " << i+1);
    }
}

