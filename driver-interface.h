#ifndef DRIVER_INTERFACE_H_INCLUDED
#define DRIVER_INTERFACE_H_INCLUDED

// This is the interface of fuctions a driver must support.

#include <time.h>

#ifndef DRIVER_IMPL
struct {} driver_device_t;
#endif

enum device_state_t {
    DEVICE_OK,
    DEVICE_DISCONNECTED,
};

// Driver state functions

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

// Create (in database) and connect to device
driver_device_t *create_device(std::string address);
driver_device_t *connect_to_device(std::string address);
void disconnect_device(driver_device_t *dev);

// the device should not be connected at this point
// this removes it from the database
void delete_device(std::string address);

std::string get_device_address(driver_device_t *dev);

device_state_t do_poll(struct timespec t, driver_device_t *dev);

#endif /* DRIVER_INTERFACE_H_INCLUDED */

