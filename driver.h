#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include <vector>
#include <string>

#include "driver-interface.h"

// This pretty much mirrors everything in driver-interface.h
struct driver_t {
    const char *(*get_name)();
    bool (*load)();
    void (*unload)();
    const char *(*error)();

    void (*add_to_util_list)(); // FIXME: real impl

    std::vector<std::string> (*discover_device_addresses)();

    driver_device_t *(*connect_to_device)(std::string address);
    void (*disconnect_device)(driver_device_t *dev);
    void (*disconnect_all_devices)();

    std::string (*get_device_address)(driver_device_t *dev);
    device_state_t (*do_poll)(struct timespec t, driver_device_t *dev);

    // probably should be "private"...
    void *lib;
};

std::vector<std::string> get_availible_drivers();

// takes name as in "faux-driver", NOT "libfaux-driver.so"
driver_t *load_driver(std::string name);
bool unload_driver(driver_t *driver);

#endif /* DRIVER_H_INCLUDED */

