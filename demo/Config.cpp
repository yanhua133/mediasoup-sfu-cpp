#include "Config.hpp"
#include <fstream>
#include "Log.hpp"
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>
#include "json.hpp"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include "windows.h"
#endif
using namespace std;
using json = nlohmann::json;
Config::Config()
{
    this->filename = "config.json";
}
Config::~Config()
{

}
void Config::getConfigData(ifstream &configfile,std::string & filedata)
{
    configfile.seekg(0,ios::end);
    auto len = configfile.tellg();  //获取文件长度
    configfile.seekg(0, ios::beg);  //设置读取位置为起始位置
    char fileBuffer[5000]={0};
    memset(fileBuffer,0,(size_t)len + 1);
    configfile.read(fileBuffer,len);
    filedata = fileBuffer;
}
string Config::TrimStringForJson(string res)
{
    for(auto iter = res.begin();iter != res.end();iter ++){

        if(isblank(*iter) != 0){       //isblank判断是否为制表符或者空格，是的话返回非零值
                                                //需包含头文件<ctype.h>

            res.erase(iter--); // --是因为重新判断一下填过来的字符

        }

    }
    //删除换行符
    int r = res.find("\n");
    while (r != string::npos)
    {
        if (r != string::npos)
        {
            res.replace(r, 1, "");
            r = res.find("\n");
            
        }
        
    }
    //删除换行符
    r = res.find("\r");
    while (r != string::npos)
    {
        if (r != string::npos)
        {
            res.replace(r, 1, "");
            r = res.find("\r");
            
        }
        
    }
    //删除换行符
    r = res.find("\t");
    while (r != string::npos)
    {
        if (r != string::npos)
        {
            res.replace(r, 1, "");
            r = res.find("\t");
            
        }
        
    }
    //删除所有空格
   // res.erase(remove_if(res.begin(),res.end(),std::isspace),res.end());
    return res;
    
}

void Config::initConfig()
{
#ifdef _WIN32
    TCHAR exeFullPath[MAX_PATH]; // Full path
    GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    char path[255] = { 0 };
    char path_buffer[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    //TcharToChar(exeFullPath, path);
    _splitpath(exeFullPath, drive, dir, fname, ext);
    strcat(path, drive);
    strcat(path, dir);
    strcat(path, "config.json");
    this->filename = path;
#else
    this->filename = "config.json";
#endif

    ifstream configfile(filename);
    //ofstream outfile("G:\\C++ project\\Read\\out.txt", ios::app);
    string line;
    if (!configfile.is_open())
    {
        MS_lOGD("not open file name =%s",filename.c_str());
        return;
    }
    std::string filedata;
    getConfigData(configfile,filedata);
    //MS_lOGD("config data=%s",filedata.c_str());
   // filedata = TrimStringForJson(filedata);
    jsonConfig = json::parse(filedata);
    //MS_lOGD("config json data=%s",jsonConfig.dump().c_str());
    //json rtmp = jsonConfig["rtmp"];
    //MS_lOGD("rtmp json data=%s",rtmp.dump().c_str());
    configfile.close();
    
    mediasoup.webRtcTransportOptions = jsonConfig["mediasoup"]["webRtcTransportOptions"];
    mediasoup.plainTransportOptions = jsonConfig["mediasoup"]["plainTransportOptions"];
    mediasoup.routerOptions = jsonConfig["mediasoup"]["routerOptions"];
    //MS_lOGD("config mediasoup.routerOptions data=%s",mediasoup.routerOptions.dump().c_str());
    this->http = jsonConfig["http"];
    this->rtp = jsonConfig["rtp"];
    this->rtmp = jsonConfig["rtmp"];


}
