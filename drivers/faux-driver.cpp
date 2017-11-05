#include <math.h>
#include <stdio.h>

#include <string>
#include <map>
#include <vector>

#include "mho-types.h"

namespace mho {

struct driver_device_t {
    device_id_t device_id; // ayy strings
    std::string address;
    double freq, amp, phase, dc;
};

} // mho

// defined this and define device_driver_t before including driver-intarface.h
#define DRIVER_IMPL
#include "driver-interface.h"

extern "C" {

static const char *driver_name = "faux-driver";

static char const *driver_error = nullptr;
static const char *ERROR_NO_MEM = "memory allocation error";

static mho::driver_id_t driver_id;

static MhoDB *_db;

const char *name(){
    return driver_name;
}

bool load(MhoDB *db){
    _db = db;
    driver_id = db->get_driver_id_or_create(name());
    return true;
}

void unload(){

}

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

mho::driver_device_t *connect_to_device(mho::device_id_t dev_id){
    mho::driver_device_t *dev = new mho::driver_device_t();
    if(!dev){
        driver_error = ERROR_NO_MEM;
        return nullptr;
    }
    dev->device_id = dev_id;
    dev->amp   = 5;
    dev->phase = 0;
    dev->dc    = 2;

    double freq, amp, phs, dc;

    mho::device_info info = _db->get_device_info(dev_id);
    int read = sscanf(info.address.c_str(), "%lf %lf %lf %lf", &freq, &amp, &phs, &dc);

    if(read-- > 0) dev->freq  = freq;
    if(read-- > 0) dev->amp   = amp;
    if(read-- > 0) dev->phase = phs;
    if(read-- > 0) dev->dc    = dc;

    return dev;
}

void disconnect_device(mho::driver_device_t *dev){
    // blah. nothing to do for this driver.
}

void disconnect_all_devices(){
    // blah. nothing to do for this driver.
}

// FIXME: delet this
std::string get_device_address(mho::driver_device_t *dev){
    //return dev->address;
    return "";
}

device_state_t do_poll(struct timespec *t, mho::driver_device_t *dev){
    mho::value_t value = dev->amp * sin(dev->freq * t->tv_sec / 6.0 + dev->phase) + dev->dc;

    _db->add_reading(dev->device_id, t, value);

    return DEVICE_OK;
}

}
