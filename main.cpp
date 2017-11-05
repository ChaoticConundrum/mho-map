#include "zlog.h"
#include "zoptions.h"

#include <thread>

#include "mhodb.h"
#include "tcpserver.h"
#include "driver.h"

using namespace LibChaos;

bool run_device_poller = true;

void device_poll_loop(void *);

int main(int argc, char **argv){
    ZLog::logLevelStdOut(ZLog::INFO, "%time% %thread% N %log%");
    ZLog::logLevelStdOut(ZLog::DEBUG, "\x1b[35m%time% %thread% D %log%\x1b[m");
    ZLog::logLevelStdErr(ZLog::ERRORS, "\x1b[31m%time% %thread% E [%function%|%file%:%line%] %log%\x1b[m");

    try {

        MhoDB db("psb.db");

        std::thread device_poller(device_poll_loop, &db);

        TCPServer server(4);
        server.run();

        device_poller.join();

    } catch(ZException e){
        ELOG("EXCEPTION: " << e.what());
    }

    return 0;
}

// FIXME THIS NEEDS TO BE MOVED TO ITS OWN CLASS
// (e.g. to add add_new_device() etc
void device_poll_loop(void *data){
    MhoDB *db = (MhoDB *)data;

    std::vector<mho::device_row> device_list = db->get_device_list();

    std::map<mho::driver_id_t, mho::driver_t *> drivers;

    // load all needed drivers (once each)
    for(auto device: device_list){
        if(drivers.find(device.driver_id) != drivers.end()) // not already loaded
            continue;

        driver_row driver_info = db->get_driver(device.driver_id);

        driver_t *driver = load_driver(driver_info.name);
        if(!driver){
            throw std::runtime_error("Could not load driver " + driver_info.name); // kys
        }

        drivers[device.driver_id] = driver;
    }

    // connect to each device
    std::map<driver_t *, std::vector<mho::driver_device_t *>> device_list_by_driver;
    for(auto device: device_list){
        driver_t *driver = drivers[device.driver_id];
        if(device_list_by_driver.find(driver) == devices.end())
            device_list_by_driver[driver] = std::vector<mho::driver_device_t *>();

        device_list_by_driver[driver].push_back(driver->connect_to_device(device));
    }

    auto start = std::chrono::high_resolution_clock::now();

    // run main poll loop
    while(run_device_poller){

        // iterate over every device for every driver
        for(auto driver: drivers){
            for(auto device: driver_list_by_driver[driver]){
                driver->do_poll(device);
            }
        }

        // sleep until 1 second after we started
        start += 1 std::chrono_literals::operator""s;
        std::this_thread::sleep_until(start);
    }

    // disconnect from each device so we can unload drivers
    for(auto driver: drivers){
        for(auto device: driver_list_by_driver[driver]){
            driver->disconnect_device(device);
        }
        driver_unload(driver);
    }

    map.clear(); // destroy the pointers, cause they got free'd in the loop above

    return;
}

