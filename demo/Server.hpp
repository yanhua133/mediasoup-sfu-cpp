#pragma once
// #include <webservice/file_request_handler.hpp>
// #include <webservice/ws_service.hpp>
// #include <webservice/ws_session.hpp>
// #include <webservice/server.hpp>
#include <boost/lexical_cast.hpp>

#include "Room.hpp"
#include "IMediasoup.hpp"
#include "IWorker.hpp"
#include "Transport/WebSocketServer.h"
#include "Config.hpp"
class MyWorkerObserver : public mediasoup::IWorker::Observer {
public:
    MyWorkerObserver() {}
    void OnSuccess() override {
        std::cout << "++++++:OnSuccess" << std::endl;
    }
    void OnFailure(std::string error) override {
        std::cout << "++++++:OnFailure" << std::endl;
    }
    void OnClose() override {
        std::cout << "++++++:OnClose" << std::endl;
    }
};

class Server
{
public:
    Server();
    ~Server();
 
public:
    void set_address(std::string ip,int port) {
        this->listenip = ip;
        this->listenport = port;
    }
    void runProtooWebSocketServer();
    std::shared_ptr<Room> getOrCreateRoom(std::string roomId);
public:
    int init();
    void run();
    void setConfig(Config &config) {
        this->config = config;
    }

    void initMediasoup();
    void initWorker(int consumerFd,int producerFd,int payloadConsumerFd,int payloadProducerFd);
    void runRawWebsocket();
    int createRawWebSocket();
    int createProtooWebSocket();
    void runRawWebsockServer();
    void processHttpRequest(std::string &path,std::string & roomId,std::shared_ptr<Room> room,json &params,json &query,json & body,json &respdata);
    std::shared_ptr<mediasoup::IWorker> getMediasoupWorker();
    json getStringFromBase64(std::string payload);
    void processRawSdpMessage(std::string message);
    void testProtoo();
private:
    //webservice::server* rawWebsockServer = nullptr;
    std::string listenip;
    int listenport;
    int thread_count=1;
    std::unordered_map<std::string,std::shared_ptr<Room> > rooms;
    std::vector<std::shared_ptr<mediasoup::IWorker>> workers;
    mediasoup::IMediasoup * mediasoup;
    mediasoup::WorkerSettings workerSettings;
    MyWorkerObserver myWorkerObserver;

    std::shared_ptr<WebSocketServer> protooWebsockServer;

    Config config;
    int nextMediasoupWorkerIdx =0;
};
