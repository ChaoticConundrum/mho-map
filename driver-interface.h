#ifndef DRIVER_INTERFACE_H_INCLUDED
#define DRIVER_INTERFACE_H_INCLUDED

// This is the interface of fuctions a driver must support.
// IF YOU ADD ANYTHING TO THIS FILE, IT SHOULD BE ADDED TO
// THE driver_t PUBLIC INTERFACE SO PEOPLE CAN USE IT (and
// populate the field in load_driver()

#include "mho-types.h"
#include <time.h>

#ifndef DRIVER_IMPL
namespace mho {
struct driver_device_t {};
} // mho
#endif

enum device_state_t {
    DEVICE_OK,
    DEVICE_DISCONNECTED,
};

// Driver state functions

// This should be the same as the shared object name. This is
// because we need to be able to find the driver to load from
// this identifier after a server restart
const char *get_name();
bool load();
void unload();

// return nullptr on no error
// clear error on check
const char *error();

void add_to_util_list(); // FIXME: real impl

// Device functions

// autodiscovery
std::vector<std::string> discover_device_addresses();

mho::driver_device_t *connect_to_device(mho::device_id_t device);
void disconnect_device(mho::driver_device_t *dev);

// for use before force-unloading a module... which isnt a great
// idea...
void disconnect_all_devices();

std::string get_device_address(mho::driver_device_t *dev);

device_state_t do_poll(struct timespec *t, mho::driver_device_t *dev);

#endif /* DRIVER_INTERFACE_H_INCLUDED */

