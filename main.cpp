#include "zlog.h"
#include "zoptions.h"

#include "devicepoller.h"
#include "mhodb.h"
#include "tcpserver.h"
#include "driver.h"

using namespace LibChaos;

int main(int argc, char **argv){
    ZLog::logLevelStdOut(ZLog::INFO, "%time% %thread% N %log%");
    ZLog::logLevelStdOut(ZLog::DEBUG, "\x1b[35m%time% %thread% D %log%\x1b[m");
    ZLog::logLevelStdErr(ZLog::ERRORS, "\x1b[31m%time% %thread% E [%function%|%file%:%line%] %log%\x1b[m");

    try {

        MhoDB *db = MhoDB::instance();

        DevicePoller device_poller(db);
        device_poller.start_loop();

        TCPServer server(4);
        server.run();

        device_poller.stop_loop();
        device_poller.join_loop();

    } catch(zexception e){
        ELOG("!!EXCEPTION!!: " << e.what);

    } catch(ZException e){
        ELOG("EXCEPTION: " << e.what());
    }

    return 0;
}

