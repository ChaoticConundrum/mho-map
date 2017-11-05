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
    int res;
    ZTable tbl;
    ZString sel = ZString("SELECT driver_id FROM drivers WHERE name = '") + name + "'";
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    if(tbl.rowCount()){
        return tbl.field("driver_id", 0).toUint();
    } else {
        return create_driver(name, "");
    }
}

//! Get driver info
driver_info MhoDB::get_driver_info(driver_id_t driver){
    int res;
    ZTable tbl;
    driver_info info;

    ZString sel = ZString("SELECT name, user_description FROM drivers WHERE driver_id = ") + driver;
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    info.driver_id = driver;
    info.name = tbl.field("name", 0).str();
    info.description = tbl.field("user_description", 0).str();

    return info;
}

// Set driver user desription
int MhoDB::set_driver_description(driver_id_t driver, string description){
    int res;
    ZString upd = ZString("UPDATE drivers SET user_description = '") + description + "' WHERE driver_id = " + driver;
    res = db.execute(upd);
    if(res != 0) ELOG("sql execute failed");

    return res;
}


//! Create device
device_id_t MhoDB::create_device(driver_id_t driver, string description, node_id_t node, value_t calibration, string address){
    int res;

    int state = (node < 1 ? mho::INACTIVE : mho::ACTIVE);

    res = db.execute(ZString("INSERT INTO devices (driver_id, description, node_id, calibration, address, state) VALUES (") + driver + ", '" + description + "', " + node + ", " + calibration + ", '" + address + "', " + state + ")");
    if(res != 0) ELOG("sql execute failed");

    ZTable tbl;
    res = db.execute("SELECT last_insert_rowid()", tbl);
    if(res != 0) ELOG("sql execute failed");

    return tbl.field(0, 0).toUint();
}

//! Get device info
device_info MhoDB::get_device_info(device_id_t device){
    int res;
    ZTable tbl;
    device_info info;

    ZString sel = ZString("SELECT driver_id, description, node_id, calibration, address, state FROM devices WHERE device_id = ") + device;
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    info.device_id = device;
    info.driver_id = tbl.field("driver_id", 0).toUint();
    info.description = tbl.field("user_description", 0).str();
    info.node_id = tbl.field("node_id", 0).toUint();
    info.calibration = std::stod(tbl.field("calibration", 0).str());
    info.address = tbl.field("address", 0).str();
    info.state = (mho::device_state)tbl.field("state", 0).tint();

    return info;
}

//! Set device description
int MhoDB::set_device_description(device_id_t device, string description){
    int res;
    ZString upd = ZString("UPDATE devices SET description = '") + description + "' WHERE device_id = " + device;
    res = db.execute(upd);
    if(res != 0) ELOG("sql execute failed");

    return res;
}

//! Set device node (or deactivate)
int MhoDB::set_device_node(device_id_t device, node_id_t node){
    int res;

    int state = (node < 1 ? mho::INACTIVE : mho::ACTIVE);

    ZString upd = ZString("UPDATE devices SET node = ") + node + ", state = " + state + " WHERE device_id = " + device;
    res = db.execute(upd);
    if(res != 0) ELOG("sql execute failed");

    return res;
}

//! Get list of non-deleted devices
vector<device_info> MhoDB::list_devices(){
    int res;
    vector<device_info> list;

    ZTable tbl;
    ZString sel = ZString("SELECT device_id, driver_id, description, node_id, calibration, address, state FROM devices WHERE state != 2");
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");
    DLOG("list_devices: " << tbl.rowCount());

    list.resize(tbl.rowCount());
    for(int i = 0; i < tbl.rowCount(); ++i){
        list[i].device_id = tbl.field("device_id", i).toUint();
        list[i].driver_id = tbl.field("driver_id", i).toUint();
        list[i].description = tbl.field("user_description", i).str();
        list[i].node_id = tbl.field("node_id", i).toUint();
        list[i].calibration = std::stod(tbl.field("calibration", i).str());
        list[i].address = tbl.field("address", i).str();
        list[i].state = (mho::device_state)tbl.field("state", i).tint();
    }

    return list;
}


//! Add reading entry
reading_id_t MhoDB::add_reading(device_id_t device, struct timespec *time, value_t raw_value){
    int res;

    ZTable tbl;
    res = db.execute(ZString("SELECT node_id, calibration FROM devices WHERE device_id = ") + device, tbl);
    if(res != 0) ELOG("sql execute failed");

    DLOG("add_reading node_id: " << tbl.field("node_id", 0));
    value_t calib = std::stod(tbl.field("calibration", 0).str());

    ZString ins = ZString("INSERT INTO readings (node_id, device_id, raw_value, adj_value, time_sec, time_nsec) VALUES (") + tbl.field("node_id", 0) + ", " + device + ", " + raw_value + ", " + raw_value * calib + ", " + time->tv_sec + ", " + time->tv_nsec + ")";
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
    int res;
    ZTable tbl;
    reading_info info;

    ZString sel = ZString("SELECT node_id, device_id, raw_value, adj_value, time_sec, time_nsec FROM readings WHERE reading_id = ") + reading;
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    info.reading_id = reading;
    info.node_id = tbl.field("node_id", 0).toUint();
    info.device_id = tbl.field("device_id", 0).toUint();
    info.raw_value = std::stod(tbl.field("raw_value", 0).str());
    info.adj_value = std::stod(tbl.field("adj_value", 0).str());
    info.time.tv_sec = tbl.field("time_sec", 0).toUint();
    info.time.tv_nsec = tbl.field("time_nsec", 0).toUint();

    return info;
}

vector<reading_info> MhoDB::filter_readings_node_time(node_id_t node, struct timespec *start, struct timespec *end){
    int res;
    ZTable tbl;
    vector<reading_info> list;

    //ZString sel = ZString("SELECT reading_id, node_id, device_id, raw_value, adj_value, time_sec, time_nsec FROM readings WHERE node_id = ") + node + " AND time_sec > " + start->tv_sec + " AND time_nsec > " + start->tv_nsec + " AND time_sec < " + end->tv_sec + " AND time_nsec < " + end->tv_nsec;
    ZString sel = ZString("SELECT reading_id, node_id, device_id, raw_value, adj_value, time_sec, time_nsec FROM readings WHERE node_id = ") + node + " AND time_sec >= " + start->tv_sec + " AND time_sec <= " + end->tv_sec;
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    list.resize(tbl.rowCount());
    for(int i = 0; i < tbl.rowCount(); ++i){
        list[i].reading_id = tbl.field("reading_id", i).toUint();
        list[i].node_id = tbl.field("node_id", i).toUint();
        list[i].device_id = tbl.field("device_id", i).toUint();
        list[i].raw_value = std::stod(tbl.field("raw_value", i).str());
        list[i].adj_value = std::stod(tbl.field("adj_value", i).str());
        list[i].time.tv_sec = tbl.field("time_sec", i).toUint();
        list[i].time.tv_nsec = tbl.field("time_nsec", i).toUint();
    }

    return list;
}


//! Create node
node_id_t MhoDB::create_node(node_id_t parent, string name, string description){
    int res;
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    ZString ins = ZString("INSERT INTO node_tree (parent_id, name, description, time_added, time_removed) VALUES (") + parent + ", '" + name + "', '" + description + "', " + ts.tv_sec + ", 0)";
    res = db.execute(ins);
    if(res != 0) ELOG("sql execute failed");

    ZTable tbl;
    res = db.execute("SELECT last_insert_rowid()", tbl);
    if(res != 0) ELOG("sql execute failed");

    return tbl.field(0, 0).toUint();
}

//! Get node info
node_info MhoDB::get_node_info(node_id_t node){
    int res;
    ZTable tbl;
    node_info info;

    ZString sel = ZString("SELECT parent_id, name, description, time_added, time_removed FROM node_tree WHERE node_id = ") + node;
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    info.node_id = node;
    info.parent_id = tbl.field("parent_id", 0).toUint();
    info.name = tbl.field("name", 0).str();
    info.description = tbl.field("description", 0).str();
    info.time_added = tbl.field("time_added", 0).toUint();
    info.time_removed = tbl.field("time_removed", 0).toUint();

    return info;
}

//! Set node name, description
int MhoDB::set_node_info(node_id_t node, string name, string description){
    int res;
    int state = (node < 1 ? mho::INACTIVE : mho::ACTIVE);

    ZString upd = ZString("UPDATE node_tree SET name = '") + name + "', description = '" + description + "' WHERE node_id = " + node;
    res = db.execute(upd);
    if(res != 0) ELOG("sql execute failed");

    return res;
}

//! Remove node
int MhoDB::remove_node(node_id_t node){
    int res;
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    ZString upd = ZString("UPDATE node_tree SET time_removed = ") + ts.tv_sec + "' WHERE node_id = " + node;
    res = db.execute(upd);
    if(res != 0) ELOG("sql execute failed");

    return res;
}

//! Get list of all nodes
vector<node_info> MhoDB::list_nodes(){
    int res;
    ZTable tbl;
    vector<node_info> list;

    ZString sel = ZString("SELECT node_id, parent_id, name, description, time_added, time_removed FROM node_tree");
    res = db.execute(sel, tbl);
    if(res != 0) ELOG("sql execute failed");

    list.resize(tbl.rowCount());
    for(int i = 0; i < tbl.rowCount(); ++i){
        list[i].node_id = tbl.field("node_id", i).toUint();
        list[i].parent_id = tbl.field("parent_id", i).toUint();
        list[i].name = tbl.field("name", i).str();
        list[i].description = tbl.field("description", i).str();
        list[i].time_added = tbl.field("time_added", i).toUint();
        list[i].time_removed = tbl.field("time_removed", i).toUint();
    }

    return list;
}


MhoDB *MhoDB::_instance = nullptr;

MhoDB *MhoDB::instance(){
    if(!_instance)
        _instance = new MhoDB("psb.db");

    return _instance;
}

