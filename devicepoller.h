#ifndef DEVICEPOLLER_H_INCLUDED
#define DEVICEPOLLER_H_INCLUDED

#include "mhodb.h"
#include "mho-types.h"
#include "driver-interface.h"
#include "driver.h"

#include <mutex>
#include <map>
#include <thread>

class DevicePoller {
public:
    DevicePoller(MhoDB *db);

    void start_loop();
    void stop_loop();
    void join_loop();

    // these will NOT unload drivers
    // unknown behavior if the device is already attached...
    void connect_device(mho::device_id_t device);
    // unknown behavior if the devices doesnt exist and your remove it
    void disconnect_device(mho::device_id_t device);

private:
    void poll_loop();
    static void start_polling(DevicePoller *);

private:
    MhoDB *_db;

    bool _run_device_poller;
    std::thread *_thread;

    std::mutex _table_lock; // protects both maps
    std::map<mho::driver_id_t, mho::driver_t *> _drivers;
    std::map<driver_t *, std::vector<mho::driver_device_t *>> _device_list_by_driver;
    std::map<mho::device_id_t, mho::driver_device_t *> _device_map;
};

#endif /* DEVICEPOLLER_H_INCLUDED */

