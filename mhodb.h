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

    //! Get ID of driver name or create driver with name and return ID
    driver_id_t get_driver_id_or_create(string driver_name);
    //!
    driver_id_t create_driver(string driver_name, string description);
    driver_info get_driver_info(driver_id_t driver_id);
    int set_driver_description(driver_id_t driver, string description);

    device_id_t create_device(driver_id_t driver, string short_name, string description, node_id_t node_id, float calibration, string address);
    //! Get list of non-deleted devices
    vector<device_info> list_devices();

    reading_id_t add_reading(device_id_t device, struct timespec time, value_t raw_value, value_t adj_value);

    node_id_t create_node(node_id_t parent, string name, string description);

private:
    ZDatabase db;
};

#endif // MHODB_H
