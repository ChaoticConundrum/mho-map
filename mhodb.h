#ifndef MHODB_H
#define MHODB_H

#include "zdatabase.h"
#include "zpath.h"

using namespace LibChaos;

class MhoDB {
public:
    MhoDB(ZPath file);

private:
    ZDatabase db;
};

#endif // MHODB_H
