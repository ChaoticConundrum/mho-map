#include "mhodb.h"

#include "zlog.h"

MhoDB::MhoDB(ZPath file){
    if(!db.open(file)){
        ELOG("failed to open database");
        return;
    }

    int res;
    ZTable tbl;

    res = db.execute("CREATE TABLE IF NOT EXISTS drivers (driver_id INTEGER PRIMARY KEY ASC, name TEXT, user_description TEXT)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS devices (device_id INTEGER PRIMARY KEY ASC, type_id INTEGER, description TEXT, node_id INTEGER, calibration REAL, address TEXT, state INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS readings (reading_id INTEGER PRIMARY KEY ASC, node_id INTEGER, device_id INTEGER, raw_value REAL, adj_value REAL, time_sec INTEGER, time_nsec INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    res = db.execute("CREATE TABLE IF NOT EXISTS node_tree (node_id INTEGER PRIMARY KEY ASC, parent_id INTEGER, name TEXT, description TEXT, time_added INTEGER, time_removed INTEGER)", tbl);
    if(res != 0){
        ELOG("create table failed");
    }

    stmt_last_id = db.prepare("SELECT last_insert_rowid()");
    if(!stmt_last_id.ok()) ELOG("failed prepare");

    stmt_create_driver = db.prepare("INSERT INTO drivers (name, user_description) VALUES (:name, :desc)");
    if(!stmt_last_id.ok()) ELOG("failed prepare");
    stmt_get_driver_info = db.prepare("SELECT driver_id, name, user_description FROM drivers WHERE driver_id IS :id");
    if(!stmt_last_id.ok()) ELOG("failed prepare");
    stmt_set_driver_info = db.prepare("UPDATE drivers SET name = :name, user_description = :desc WHERE driver_id = :id");
    if(!stmt_last_id.ok()) ELOG("failed prepare");

}

//! Create new driver
driver_id_t MhoDB::create_driver(string name, string description){
    int res;
    LOG("1");
    res = stmt_create_driver.bind(1, name);
    if(res != 0) ELOG("sql bind failed");
    //LOG(sqlite3_errmsg(db.handle()));

    LOG("2");
    res = stmt_create_driver.bind(2, description);
    if(res != 0) ELOG("sql bind failed");
    LOG("3");

    ZTable tbl;
    res = stmt_create_driver.execute(tbl);
    if(res != 0) ELOG("sql execute failed");
    LOG("rows: " << tbl.rowCount());

    res = stmt_last_id.execute(tbl);
    if(res != 0) ELOG("sql execute failed");
    LOG("rows: " << tbl.rowCount());

    return 0;
}

//! Get ID of driver name or create driver with name and return ID
driver_id_t MhoDB::get_driver_id_or_create(string name){

    return 0;
}

//! Get driver info
driver_info MhoDB::get_driver_info(driver_id_t driver){
    driver_info info;

    return info;
}

// Set driver user desription
int MhoDB::set_driver_description(driver_id_t driver, string description){

    return -1;
}


//! Create device
device_id_t MhoDB::create_device(driver_id_t driver, string description, node_id_t node, value_t calibration, string address){
    return 0;
}

//! Get device info
device_info MhoDB::get_device_info(device_id_t device){
    device_info info;

    return info;
}

//! Set device description
int MhoDB::set_device_description(device_id_t device, string description){
    return -1;
}

//! Set device node (or deactivate)
int MhoDB::set_device_node(device_id_t device, node_id_t node){
    return -1;
}

//! Get list of non-deleted devices
vector<device_info> MhoDB::list_devices(){
    vector<device_info> list;

    return list;
}


//! Add reading entry
reading_id_t MhoDB::add_reading(device_id_t device, struct timespec time, value_t raw_value){
    return 0;
}

//! Get reading info
reading_info MhoDB::get_reading_info(reading_id_t reading){
    reading_info info;

    return info;
}


//! Create node
node_id_t MhoDB::create_node(node_id_t parent, string name, string description){
    return 0;
}

//! Get node info
node_info MhoDB::get_node_info(node_id_t node){
    node_info info;

    return info;
}

//! Set node name, description
int MhoDB::set_node_info(node_id_t, string name, string description){
    return -1;
}

//! Remove node
int MhoDB::remove_node(node_id_t node){
    return -1;
}

