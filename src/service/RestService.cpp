#include "iostream"
#include <httplib.h>
#include "yaml-cpp/yaml.h"
#include "RestService.h"
#include "../json.hpp"
#include "../core/SpatialHelper.h"

using namespace std;
using namespace nlohmann;
using namespace httplib;

#define TIME_OUT_SEC  300

static int port_;
static map<string, string> fieldMap_;

std::string getNowTime()
{
    /* 获取时间 */
    time_t time_seconds = time(nullptr);
    struct tm now_time{};
    localtime_s(&now_time, &time_seconds);
    char timeBuf[64] = {0};
    strftime(timeBuf, 64, "%Y-%m-%d %H:%M:%S", &now_time);
    return timeBuf;
}

void RestService::start() {
    Server svr;
    svr.set_write_timeout(TIME_OUT_SEC);
    svr.set_read_timeout(TIME_OUT_SEC);

    svr.Get("/hi", [](const Request& req, Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.Post("/processors/pipeline_distance", [](const Request &req, Response &resp){
        cout << "last request time : " << getNowTime() << endl;

        string requestStr = req.body;
        json jReq = json::parse(requestStr);

        /**
         例子:
           {
                "doType": 1,          1 = 水平  2=垂直  3=两值都要
                "localSource": {
                    "data": "E:/testdata/pipeline/wxJSPipe.gdb",
                    "point": "JSPOINT",
                    "line": "JSLINE",
                    "lineIdField": "OBJECTID",
                    "idList": [25148, 25136]
                },
                "localTarget": {
                    "data": "E:/testdata/pipeline/wxWSPipe.gdb",
                    "point": "WSPOINT",
                    "line": "WSLINE",
                    "lineIdField": "OBJECTID",
                    "idList": [10621, 10623, 10624]
                }
           }
         */

        try {
            int do_type = jReq["doType"].get<int>();
            string data_path = jReq["localSource"]["data"].get<string>();
            string point_layer = jReq["localSource"]["point"].get<string>();
            string line_layer = jReq["localSource"]["line"].get<string>();
            string line_layer_id_field = jReq["localSource"]["lineIdField"].get<string>();
            vector<long> id_list = jReq["localSource"]["idList"].get<vector<long>>();

            string data_path_other = jReq["localTarget"]["data"].get<string>();
            string point_layer_other = jReq["localTarget"]["point"].get<string>();
            string line_layer_other = jReq["localTarget"]["line"].get<string>();
            string line_layer_id_field_other = jReq["localTarget"]["lineIdField"].get<string>();
            vector<long> id_list_other = jReq["localTarget"]["idList"].get<vector<long>>();

            Field_Attr field_attr(line_layer_id_field,
                                  fieldMap_["objectNumField"],
                                  fieldMap_["startAltitude"],
                                  fieldMap_["endAltitude"],
                                  fieldMap_["startObjectNumField"],
                                  fieldMap_["endObjectNumField"],
                                  fieldMap_["pipeDiameter"]
                                  );
            // 生成参数
            Data_Attr data_attr(data_path, point_layer, line_layer,
                                data_path_other,point_layer_other,line_layer_other);

            SpatialHelper spatial_helper(field_attr, data_attr);
            spatial_helper.setObjectIdList(id_list);
            spatial_helper.setObjectIdOtherList(id_list_other);

            // 执行
            auto t = (do_distance_type)do_type;
            spatial_helper.run(t);

            // 获取结果
            string result = spatial_helper.getResult();

            // 返回
            resp.set_content(result, "application/json");

        }catch (const exception& e){
            printf("service error = %s\n", e.what());
        }
    });

    if (!RestService::init()){
        return;
    }

    std::cout << "server start......  port : " << port_ << std::endl;
    svr.listen("0.0.0.0", port_);
}

bool RestService::init() {
    YAML::Node config = YAML::LoadFile("config.yaml");
    try {
        port_ = config["port"].as<int>();
        fieldMap_["objectNumField"] = config["pointLayer"]["objectNumField"].as<string>();
        fieldMap_["startObjectNumField"] = config["lineLayer"]["startObjectNumField"].as<string>();
        fieldMap_["endObjectNumField"] = config["lineLayer"]["endObjectNumField"].as<string>();
        fieldMap_["startAltitude"] = config["lineLayer"]["startAltitude"].as<string>();
        fieldMap_["endAltitude"] = config["lineLayer"]["endAltitude"].as<string>();
        fieldMap_["pipeDiameter"] = config["lineLayer"]["pipeDiameter"].as<string>();

        return true;
    }
    catch (const exception& e) {
        cout << "init failed! " << e.what() << endl;
        return false;
    }
}
