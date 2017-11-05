#include "zlog.h"
#include "zoptions.h"

#include "tcpserver.h"

using namespace LibChaos;

int main(int argc, char **argv){
    ZLog::logLevelStdOut(ZLog::INFO, "%time% %thread% N %log%");
    ZLog::logLevelStdOut(ZLog::DEBUG, "\x1b[35m%time% %thread% D %log%\x1b[m");
    ZLog::logLevelStdErr(ZLog::ERRORS, "\x1b[31m%time% %thread% E [%function%|%file%:%line%] %log%\x1b[m");

    TCPServer server(4);
    server.run();

    return 0;
}

