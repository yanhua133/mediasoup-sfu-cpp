//
// Created by pan chenguang on 2021/2/4.
//

#include "Message.h"
#include "../utils/util.h"

using namespace std;
using namespace nlohmann;

#define PROTOO_LOG_ENABLE 0

json Message::parse(string raw) {
    //get json object from raw string
    auto object = json::parse(raw);
    auto message = json({});
    if(object.contains("request")){
        if (object["request"].is_boolean()) {
            message["request"] = true;
            message["id"] = object["id"].get<int>();
            message["method"] = object["method"].get<std::string>();
            message["data"] = object["data"];
        }
    }else if(object.contains("response")){
        if (object["response"].is_boolean()) {
            message["response"] = true;
            message["id"] = object["id"].get<int>();
            if (object["ok"].is_boolean()) {
                if (object["ok"]) {
                    message["ok"] = true;
                    message["data"] = object["data"];
                }
                else {
                    message["ok"] = false;
                    message["errorCode"] = object["errorCode"].get<int>();
                    message["errorReason"] = object["errorReason"].get<std::string>();
                }
            }
        }
    }else if(object.contains("notification")){
        if (object["notification"].is_boolean()) {
            message["notification"] = true;
            message["id"] = object["id"];
            message["method"] = object["method"];
            message["data"] = object["data"];
            
        }
    }else{
        //invalid
        //return nullprt;  //json::is_null()
        //return json({});//empty json can check through  or empty()
    }
#if PROTOO_LOG_ENABLE
    std::cout << "[Message] parsed message=" << message << std::endl;
#endif
    return message;
}
json Message::createRequest(string method, json data) {
    json request = {
        {"request", true},
        {"id", generateRandomNumber()},
        {"method", method},
        {"data", data}
    };
#if PROTOO_LOG_ENABLE
    std::cout << "[Message] createRequest request=" << request << std::endl;
#endif
    
    return request;
}
json Message::createSuccessResponse(json request, json data) {
    json response = {
        {"response", true},
        {"id", request["id"].get<int>()},
        {"ok", true},
        {"data", data}
    };
#if PROTOO_LOG_ENABLE
    std::cout << "[Message] createSuccessResponse response=" << response << std::endl;
#endif
    
    return response;
}
json Message::createErrorResponse(json request, int errorCode, string errorReason) {
    json response = {
        {"response", true},
        {"id", request["id"].get<int>()},
        {"ok", false},
        {"errorCode", errorCode},
        {"errorReason", errorReason},
    };
#if PROTOO_LOG_ENABLE
    std::cout << "[Message] createErrorResponse response=" << response << std::endl;
#endif
    
    return response;
    
}
json Message::createNotification(string method, json data) {
    json notification = {
        {"notification", true},
        {"method", method},
        {"data", data}
    };
#if PROTOO_LOG_ENABLE
    std::cout << "[Message] createNotification notification=" << notification << std::endl;
#endif
    
    return notification;
}
