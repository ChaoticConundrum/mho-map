#ifndef MHODB_H
#define MHODB_H

#include "mho-types.h"

#include "zdatabase.h"
#include "zpath.h"

#include <vector>

using namespace mho;
using namespace LibChaos;

class MhoDB {
public:
    MhoDB(ZPath file);

    //! Create new driver
    driver_id_t create_driver(string name, string description);
    //! Get ID of driver name or create driver with name and return ID
    driver_id_t get_driver_id_or_create(string name);
    //! Get driver info
    driver_info get_driver_info(driver_id_t driver);
    // Set driver user desription
    int set_driver_description(driver_id_t driver, string description);

    //! Create device
    device_id_t create_device(driver_id_t driver, string description, node_id_t node, value_t calibration, string address);
    //! Get device info
    device_info get_device_info(device_id_t device);
    //! Set device description
    int set_device_description(device_id_t device, string description);
    //! Set device node (or deactivate)
    int set_device_node(device_id_t device, node_id_t node);
    //! Get list of non-deleted devices
    vector<device_info> list_devices();

    //! Add reading entry
    reading_id_t add_reading(device_id_t device, struct timespec time, value_t raw_value);
    //! Get reading info
    reading_info get_reading_info(reading_id_t reading);

    //! Create node
    node_id_t create_node(node_id_t parent, string name, string description);
    //! Get node info
    node_info get_node_info(node_id_t node);
    //! Set node name, description
    int set_node_info(node_id_t, string name, string description);
    //! Remove node
    int remove_node(node_id_t node);

private:
    ZDatabase db;
};

#endif // MHODB_H
