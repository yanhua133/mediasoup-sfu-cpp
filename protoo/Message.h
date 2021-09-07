//
// Created by pan chenguang on 2021/2/4.
//

#ifndef TESTBEASTCLION_MESSAGE_H
#define TESTBEASTCLION_MESSAGE_H

#include <string>
#include "utils/json.hpp"

using namespace std;
using namespace nlohmann;
namespace protoo {
    class Message{
    public:
        static json parse(string raw);
        static json createRequest(string method,json data);
        static json createSuccessResponse(json request,json data);
        static json createErrorResponse(json request,int errorCode,string errorReason);
        static json createNotification(string method, json data);
    };

}


#endif //TESTBEASTCLION_MESSAGE_H
