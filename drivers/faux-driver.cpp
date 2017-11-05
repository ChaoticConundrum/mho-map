#include <math.h>

#include <string>
#include <map>
#include <vector>

#include "mho-types.h"

struct driver_device_t {
    std::string node_id; // ayy strings
    std::string address;
};

// defined this and define device_driver_t before including driver-intarface.h
#define DRIVER_IMPL
#include "driver-interface.h"

static const char *driver_name = "faux-driver";

static char const *driver_error = nullptr;
static const char *ERROR_NO_MEM = "memory allocation error";

static mho::driver_id_t driver_id;

const char *name(){
    return driver_name;
}

bool load(){
    // FIXME: db
    // driver_id = db.get_driver_id_or_create(name());
}

void unload(){}

const char *error(){
    const char *error = driver_error;
    driver_error = nullptr;
    return error;
}

void add_to_util_list(){
    // FIXME: real impl
    // util.register_call(...);
}

std::vector<std::string> discover_device_addresses(){
    return std::vector<std::string>();
}

driver_device_t *connect_to_device(mho::node_id_t dev_id){
    driver_device_t *dev = new driver_device_t();
    if(!dev){
        driver_error = ERROR_NO_MEM;
    return nullptr;
    }

    return nullptr;
}

void disconnect_device(driver_device_t *dev){
    // blah. nothing to do for this driver.
}

void disconnect_all_devices(){
    // blah. nothing to do for this driver.
}

std::string get_device_address(driver_device_t *dev){
    return dev->address;
}

device_state_t do_poll(struct timespec t, driver_device_t *dev){
    mho::value_t value = 5 * sin(t.tv_sec) + 2;

    // FIXME: DB
    // db.add_reading(dev->device_id, t, raw_value, adj_value,

    return DEVICE_OK;
}

