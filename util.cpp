
#include "util.h"
#include "mhodb.h"

#define RETURN_INVALID_JSON(message) do { \
resp["status"] = "failure"; \
resp["reason"] = message \
return resp; \
} while(0)

using namespace LibChaos;

namespace util {

ZJSON call(ZJSON input){
    ZJSON json(ZJSON::OBJECT);
    if(!json.object().contains("func") || json["func"].type() != ZJSON::STRING
        || !json.object().contains("seq") || json["seq"].type() != ZJSON::NUMBER){
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

    // FIXME: offload into util.call(string map)
    if(func == "version"){
        resp_data["version"] = "0.2.0";
    } else if(func == "fetch_data_by_node"){
        // check if we have valid id list etc
        for(size_t i = 0; i < json["args"]["ids"].array().size(); ++i){
            // FIXME: verify type of json object is correct
            ZJSON valueArray(ZJSON::ARRAY);
            // fetch data from db
            for(int j = 0; j < 2; ++i){
                ZJSON data(ZJSON::OBJECT);
                data["time"] = ((double)time(nullptr)) + j/2;
                data["power"] = 5; // sin(time(nullptr)/3600.0) + 2.0;
                valueArray[i] = data;
            }
            resp_data[ZString((int)json["args"]["ids"][i].number())] = valueArray;
        }
        // FIXME: call driver
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
    } else if(func == "get_all_nodes"){
    }

    resp["resp"] = resp_data;

    return resp;
}

} // util

