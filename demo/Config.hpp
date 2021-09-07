#pragma once
#include <fstream>
#include "Log.hpp"
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
struct mediasoup_
{
    json webRtcTransportOptions;
    json plainTransportOptions;
    json routerOptions;

};

class Config
{
public:
    Config();
    ~Config();
    void initConfig();
    mediasoup_ mediasoup;
    json http;
    json rtp;
    json rtmp;
    json jsonConfig;
    void getConfigData(ifstream &configfile,std::string & filedata);
    string TrimStringForJson(string raw);
private:
    std::string filename = "config.json";
    
};
