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

    res = db.execute("CREATE TABLE IF NOT EXISTS devices (device_id INTEGER PRIMARY KEY ASC, driver_id INTEGER, description TEXT, node_id INTEGER, calibration REAL, address TEXT, state INTEGER)", tbl);
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

    //res = db.execute("SELECT COUNT(*) FROM node_tree", tbl);

    //stmt_create_driver = db.prepare("INSERT INTO drivers (name, user_description) VALUES (:name, :desc)");
    //stmt_get_driver_info = db.prepare("SELECT driver_id, name, user_description FROM drivers WHERE driver_id IS :id");
    //stmt_set_driver_info = db.prepare("UPDATE drivers SET name = :name, user_description = :desc WHERE driver_id = :id");

}

//! Create new driver
driver_id_t MhoDB::create_driver(string name, string description){
    int res;

    res = db.execute(ZString("INSERT INTO drivers (name, user_description) VALUES ('") + name + "', '" + description +"')");
    if(res != 0) ELOG("sql execute failed");

    ZTable tbl;
    res = db.execute("SELECT last_insert_rowid()", tbl);
    if(res != 0) ELOG("sql execute failed");

    return tbl.field(0, 0).toUint();
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
    int res;

    int state = (node < 0 ? mho::INACTIVE : mho::ACTIVE);

    res = db.execute(ZString("INSERT INTO devices (driver_id, description, node_id, calibration, address, state) VALUES (") + driver + ", '" + description + "', " + node + ", " + calibration + ", '" + address + "', " + state + ")");
    if(res != 0) ELOG("sql execute failed");

    ZTable tbl;
    res = db.execute("SELECT last_insert_rowid()", tbl);
    if(res != 0) ELOG("sql execute failed");

    return tbl.field(0, 0).toUint();
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
    int res;

    ZTable tbl;
    res = db.execute(ZString("SELECT node_id, calibration FROM devices WHERE device_id = ") + device, tbl);

    DLOG("add_reading node_id: " << tbl.field("node_id", 0));
    value_t calib = std::stod(tbl.field("calibration", 0).str());

    ZString ins = ZString("INSERT INTO readings (node_id, device_id, raw_value, adj_value, time_sec, time_nsec) VALUES (") + tbl.field("node_id", 0) + ", " + device + ", " + raw_value + ", " + raw_value * calib + ", " + time.tv_sec + ", " + time.tv_nsec + ")";
    //LOG(ins);
    res = db.execute(ins);
    if(res != 0) ELOG("sql execute failed");
    //LOG(sqlite3_errmsg(db.handle()));

    ZTable tbl3;
    res = db.execute("SELECT last_insert_rowid()", tbl3);
    if(res != 0) ELOG("sql execute failed");

    return tbl3.field(0, 0).toUint();
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

