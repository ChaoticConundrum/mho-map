#include "devicepoller.h"

#include <algorithm>

DevicePoller::DevicePoller(MhoDB *db):
    _db(db),
    _run_device_poller(true),
    _thread(nullptr)
{
}

void DevicePoller::start_loop(){
    if(_thread) // aww heeeelllllll naw
        return;

    _thread = new std::thread(start_polling, this);
}

void DevicePoller::stop_loop(){
    _run_device_poller = false;
}

void DevicePoller::join_loop(){
    _thread->join();
    delete _thread;
    _thread = nullptr;
}

void DevicePoller::start_polling(DevicePoller *dp){
    dp->poll_loop();
}

void DevicePoller::connect_device(mho::device_id_t device){
    _table_lock.lock();

    driver_info driver_row = _db->get_driver_info(device);

    if(_drivers.find(driver_row.driver_id) == _drivers.end()){
        // module not loaded
        _drivers[driver_row.driver_id] = load_driver(driver_row.name);
    }

    driver_t *driver = _drivers[driver_row.driver_id];

    mho::driver_device_t *drv_dev = driver->connect_to_device(device);

    _device_list_by_driver[driver].push_back(drv_dev);
    _device_map[device] = drv_dev;

    _table_lock.unlock();
}

void DevicePoller::disconnect_device(mho::device_id_t device){
    _table_lock.lock();

    driver_info driver_row = _db->get_driver_info(device);

    driver_t *driver = _drivers[driver_row.driver_id];
    driver_device_t *drv_dev = _device_map[device];

    // disconnect
    driver->disconnect_device(drv_dev);

    // remove entries
    _drivers.erase(device);
    _device_map.erase(device);

    // find entry...
    auto vec = _device_list_by_driver[driver];
    auto it = std::find(vec.begin(), vec.end(), drv_dev);
    vec.erase(it);

    _table_lock.unlock();
}

void DevicePoller::poll_loop(){
    _table_lock.lock();

    std::vector<mho::device_info> device_list = _db->list_devices();

    // load all needed drivers (once each)
    for(auto device: device_list){
        if(_drivers.find(device.driver_id) != _drivers.end()) // not already loaded
            continue;

        driver_info driver_row = _db->get_driver_info(device.driver_id);

        driver_t *driver = load_driver(driver_row.name);
        if(!driver){
            throw std::runtime_error("Could not load driver " + driver_row.name); // kys
        }

        _drivers[device.driver_id] = driver;
    }

    // connect to each device
    for(auto device: device_list){
        driver_t *driver = _drivers[device.driver_id];
        if(_device_list_by_driver.find(driver) == _device_list_by_driver.end())
            _device_list_by_driver[driver] = std::vector<mho::driver_device_t *>();

        mho::driver_device_t *drv_dev = driver->connect_to_device(device.device_id);

        _device_list_by_driver[driver].push_back(drv_dev);
        _device_map[device.device_id] = drv_dev;
    }

    _table_lock.unlock();

    auto start = std::chrono::high_resolution_clock::now();

    // run main poll loop
    while(_run_device_poller){
        _table_lock.lock();

        struct timespec loop_time;
        clock_gettime(CLOCK_REALTIME, &loop_time);

        // iterate over every device for every driver
        for(auto driver: _drivers){
            for(auto device: _device_list_by_driver[driver.second]){
                driver.second->do_poll(&loop_time, device);
            }
        }

        _table_lock.unlock();

        // sleep until 1 second after we started
        { using namespace std::chrono_literals; 
            start += 1s; }
        std::this_thread::sleep_until(start);
    }

    _table_lock.lock();

    // disconnect from each device so we can unload drivers
    for(auto driver: _drivers){
        for(auto device: _device_list_by_driver[driver.second]){
            driver.second->disconnect_device(device);
        }
        mho::unload_driver(driver.second);
        // POINTERS ARE NOW INVALID
    }

    _drivers.clear();
    _device_list_by_driver.clear(); // destroy the pointers, cause they got free'd in the loop above
    _device_map.clear();

    _table_lock.unlock();

    return;
}


