#ifndef MHO_TYPES_H_INCLUDED
#define MHO_TYPES_H_INCLUDED

// I love bloat!
#include <stdint.h>

namespace mho {

typedef uint32_t node_id_t;
typedef uint32_t device_id_t;
typedef uint32_t type_id_t;

typedef double value_t;
typedef double calibration_t;

// use struct timespec for times.
// { .tv_sec, .tv_nsec }

}; // mho

#endif /* MHO_TYPES_H_INCLUDED */
