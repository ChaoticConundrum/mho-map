#include "netthread.h"
#include "tcpserver.h"
#include "connection.h"

#include "zlog.h"
#include "zjson.h"

#include <unistd.h>

void sendData(ZPointer<Connection> conn, const ZBinary &data){
    // Send data
    conn->out_buffer.concat(data);

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
}

void sendJSON(ZPointer<Connection> conn, ZJSON json){
    sendData(conn, json.encode() + "\r\n");
}

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

        try {

            switch(conn->status){
                case Connection::READ: {
                    //RLOG(conn->in_buffer.dumpBytes(1, 32));

                    ZString str(conn->in_buffer.raw(), conn->in_buffer.size());
                    str.strip('\n');
                    str.strip('\r');
                    str.strip('\n');

                    if(str.size() == 0){
                        conn->in_buffer.clear();
                        break;
                    }

                    ZJSON json;
                    if(!json.decode(str) || json.type() != ZJSON::OBJECT){
                        ELOG("invalid request json");
                        conn->in_buffer.clear();
                        ZJSON resp(ZJSON::OBJECT);
                        resp["error"] = "invalid json";
                        sendJSON(conn, resp);
                        break;
                    }
                    conn->in_buffer.clear();

                    if(!json.object().contains("func") || json["func"].type() != ZJSON::STRING
                        || !json.object().contains("seq") || json["seq"].type() != ZJSON::NUMBER){
                        ELOG("invalid request fields (need \"func\" and \"seq\")");
                        ZJSON resp(ZJSON::OBJECT);
                        resp["error"] = "invalid fields";
                        sendJSON(conn, resp);
                        break;
                    }

                    ZString func = json["func"].string();
                    LOG(str);
                    LOG("Function: " << func);

                    ZJSON resp(ZJSON::OBJECT);
                    resp["seq"] = json["seq"];
                    ZJSON resp_data(ZJSON::OBJECT);

                    // FIXME: offload into util.call(string map)
                    if(func == "version"){
                        resp_data["version"] = "0.1.0";
                    } else if(func == "fetch_data_by_node"){
                        // check if we have valid id list etc
                        // for
                        // resp_data[""];
                        // FIXME: call driver
                    }

                    resp["resp"] = resp_data;

                    sendJSON(conn, resp);
                    break;
                }

                case Connection::WRITE:
                    break;

                default:
                   ELOG("Bad connection status");
                   conn->connection->close();
                   break;
            }

        } catch(ZException e){
            ELOG("EXCEPTION: " << e.what());
        }

        conn->lock.unlock();
    }

    return nullptr;
}
