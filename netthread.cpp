#include "netthread.h"
#include "tcpserver.h"
#include "connection.h"

#include "zlog.h"

#include <unistd.h>

NetThread::NetThread(){

}

void *NetThread::run(void *arg){
    TCPServer *server = (TCPServer *)arg;
    ZPointer<Connection> conn;
//    LOG("Thread started");

    while(true){
        // Wait for work
        conn = server->queue.getWork();

        // Check for null job
        if(conn.get() == nullptr){
            server->queue.addWork(nullptr);
            break;
        }

        conn->lock.lock();

        switch(conn->status){
            case Connection::READ: {
                LOG("In: " << conn->in_buffer.size() << " bytes");
                ZString str(conn->in_buffer.raw(), conn->in_buffer.size());
                ZJOSN json;
                if(!json.decode(str)){

                    break;
                }


                // Free resources
                conn->in_buffer.clear();

                // Dummy data
                ZBinary data("OK", 2);
                conn->out_buffer.concat(data);

                LOG("Out: " << conn->out_buffer.size() << " bytes");
                //RLOG(conn->out_buffer.dumpBytes(1, 32));

                conn->final = true;
                conn->status = Connection::WRITE;

                // bump the out event
                zbyte count[8];
                ZBinary::enczu64(count, 1);
                long ret = write(conn->out_eventfd, count, 8);
                if(ret != 8){
                    ELOG("write error");
                }

                break;
            }

            case Connection::WRITE:
                break;

            default:
                ELOG("Bad connection status");
                conn->connection->close();
                break;
        }

        conn->lock.unlock();
    }

    return nullptr;
}
