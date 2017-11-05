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

        //driver_id_t dr = db.create_driver("test", "test driver");
        //device_id_t dev = db.create_device(dr, "test device", 0, 1.0f, "local");
        //struct timespec ts;
        //timespec_get(&ts, TIME_UTC);
        //reading_id_t r = db.add_reading(dev, &ts, 4.2645);
        //LOG("reading: " << r);

        DevicePoller device_poller(db);
        device_poller.start_loop();

        device_poller.connect_device(16);

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

