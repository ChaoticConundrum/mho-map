
#include "util.h"
#include "mhodb.h"

#include <sstream>
#include <string>
#include <iomanip>

#define RETURN_INVALID_JSON(message) do { \
resp["status"] = "failure"; \
resp["reason"] = message ; \
return resp; \
} while(0)

using namespace LibChaos;

namespace util {

std::string timespec_to_string(struct timespec *t){
    std::stringstream ss;
    ss << t->tv_sec << '.'
       << std::setfill('0') << std::setw(9) << t->tv_nsec;
    return ss.str();
}

ZJSON call(ZJSON json){
    if(!json.object().contains("func") || json["func"].type() != ZJSON::STRING
        || !json.object().contains("seq") || json["seq"].type() != ZJSON::NUMBER){
        ELOG("1:" << (!json.object().contains("func"))
          << " 2:" << (json["func"].type() != ZJSON::STRING)
          << " 3:" << (!json.object().contains("seq"))
          << " 4:" << (json["seq"].type() != ZJSON::NUMBER));
        ELOG("invalid request fields (need \"func\" and \"seq\")");
        ZJSON resp(ZJSON::OBJECT);
        resp["error"] = "invalid fields";

        return resp;
    }

    ZString func = json["func"].string();
    LOG(":: util::call: function: " << func);

    ZJSON resp(ZJSON::OBJECT);
    resp["seq"] = json["seq"];
    resp["status"] = "success";
    ZJSON resp_data(ZJSON::OBJECT);

    MhoDB *db = MhoDB::instance();

    if(func == "version"){
        resp_data["version"] = "0.2.0";

    } else if(func == "get_data_range"){
        // check if we have valid id list etc
        
        struct timespec ts_start, ts_end;
        double start = json["args"]["start"].number();
        double end   = json["args"]["end"].number();

        ts_start.tv_sec  = (int)start;
        ts_start.tv_nsec = (start - (double)(int)start) * 1000000;

        ts_end.tv_sec  = (int)end;
        ts_end.tv_nsec = (end - (double)(int)end) * 1000000;

        LOG("s: " << timespec_to_string(&ts_start) <<
          "  e: " << timespec_to_string(&ts_end));

        // for each node
        for(size_t i = 0; i < json["args"]["ids"].array().size(); ++i){
            mho::node_id_t node_id = (int)json["args"]["ids"][i].number();
            std::vector<mho::reading_info> readings = db->filter_readings_node_time(node_id, &ts_start, &ts_end);

            ZJSON array(ZJSON::ARRAY);
            // for every entry
            for(auto datapoint: readings){
                ZJSON data(ZJSON::OBJECT);
                data["node_id"]   = datapoint.node_id;
                data["time"]      = ZString(timespec_to_string(&datapoint.time));
                data["power"]     = datapoint.adj_value;
                data["raw_value"] = datapoint.raw_value;
                array.array().push(data);
            }
            resp_data[ZString(node_id)] = array;
        }

    } else if(func == "get_all_nodes"){
        std::vector<mho::node_info> nodes = db->list_nodes();
        for(auto node: nodes){
            ZJSON node_data(ZJSON::OBJECT);

            node_data["node_id"]      = node.node_id;
            node_data["parent_id"]    = node.parent_id;
            node_data["description"]  = ZString(node.description);
            node_data["time_added"]   = ZJSON((double)node.time_added);
            node_data["time_removed"] = ZJSON((double)node.time_removed);

            resp_data[ZString(node.node_id)] = node_data;
        }

    } else if(func == "get_all_devices"){
        std::vector<mho::device_info> devices = db->list_devices();
        for(auto device: devices){
            ZJSON device_data(ZJSON::OBJECT);

            device_data["device_id"]   = (double)device.device_id;
            device_data["driver_id"]   = (double)device.driver_id;
            device_data["node_id"]     = (double)device.node_id;
            device_data["description"] = ZString(device.description);
            device_data["calibration"] = (double)device.calibration;
            device_data["address"]     = ZString(device.address);
            device_data["device_state"] = (double)device.state;

            resp_data[ZString(device.device_id)] = device_data;
        }

    } else if(func == "get_all_drivers"){
        std::vector<mho::driver_info> drivers = db->list_drivers();
        for(auto driver: drivers){
            ZJSON driver_data(ZJSON::OBJECT);

            driver_data["driver_id"]   = (double)driver.driver_id;
            driver_data["name"]        = ZString(driver.name);
            driver_data["driver_description"] = ZString(driver.description);
            driver_data["user_description"]   = ZString(driver.description);

            resp_data[ZString(driver.driver_id)] = driver_data;
        }

    } else if(func == "create_device"){
        mho::driver_id_t driver_id = (uint32_t)json["args"]["driver_id"].number();
        std::string description    = json["args"]["description"].string().str();
        mho::node_id_t  node_id    = (int)json["args"]["node_id"].number();
        double calibration         = json["args"]["calibration"].number();
        std::string address        = json["args"]["address"].string().str();

        mho::device_id_t dev_id = db->create_device(driver_id, description, node_id, calibration, address);
        json["args"]["device_id"] = (double)dev_id;

        resp_data = json["args"];
    } else {
        RETURN_INVALID_JSON("unknown command");
    }

    resp["resp"] = resp_data;

    return resp;
}

} // util

