#ifndef MHO_TYPES_H_INCLUDED
#define MHO_TYPES_H_INCLUDED

#include <stdint.h>
#include <time.h>
#include <string>

using namespace std;

namespace mho {

typedef uint32_t driver_id_t;
typedef uint32_t device_id_t;
typedef uint64_t reading_id_t;
typedef int32_t node_id_t;

typedef double value_t;

enum device_state {
    INACTIVE = 0,
    ACTIVE = 1,
    DELETED = 2,
};

struct driver_info {
    driver_id_t type_id;
    string name;
    string description;
};

struct device_info {
    device_id_t device_id;
    driver_id_t driver_id;
    string description;
    node_id_t node_id;
    value_t calibration;
    string address;
    device_state state;
};

struct reading_info {
    reading_id_t reading_id;
    node_id_t node_id;
    value_t raw_value;
    value_t adj_value;
    struct timespec time;
};

struct node_info {
    node_id_t node_id;
    node_id_t parent_id;
    string name;
    string description;
    uint32_t time_added;
    uint32_t time_removed;
};

} // mho

#endif /* MHO_TYPES_H_INCLUDED */
