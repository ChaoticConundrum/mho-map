#ifndef CONNECTION_H
#define CONNECTION_H

#include "zbinary.h"
#include "zpointer.h"
#include "zmutex.h"
#include "zconnection.h"

using namespace LibChaos;

class Connection {
public:
    enum ConnectionStatus {
        NEW,        //!< New connection.
        READ,       //!< New request data read.
        SERVE,      //!< Currently serving a request (may involve more reading / writing).
        WRITE,      //!< Data pending write.
        DONE,       //!< Request completed.
        CLOSE,      //!< Connection closed.
    };

public:
    ZMutex lock;
    ZPointer<ZConnection> connection;
    int out_eventfd;
    ConnectionStatus status;

    ZBinary in_buffer;
    ZBinary out_buffer;
    bool final;
};

#endif // CONNECTION_H
