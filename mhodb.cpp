#include "mhodb.h"

#include "zlog.h"

MhoDB::MhoDB(ZPath file){
    if(!db.open(file)){
        ELOG("failed to open database");
        return;
    }

    int res;
    ZTable tbl;

    res = db.execute("CREATE TABLE IF NOT EXISTS drivers (type_id INTEGER PRIMARY KEY ASC, name TEXT, user_description TEXT)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS devices (device_id INTEGER PRIMARY KEY ASC, type_id INTEGER, description TEXT, node_id INTEGER, calibration REAL, address TEXT, state INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS readings (reading_id INTEGER PRIMARY KEY ASC, node_id INTEGER, raw_value REAL, adj_value REAL, time_sec INTEGER, time_nsec INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS node_tree (node_id INTEGER PRIMARY KEY ASC, parent_id INTEGER, name TEXT, description TEXT, time_added INTEGER, time_removed INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }
}




