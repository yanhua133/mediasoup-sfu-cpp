#pragma once
// #include <webservice/file_request_handler.hpp>
// #include <webservice/ws_service.hpp>
// #include <webservice/ws_session.hpp>
// #include <webservice/server.hpp>
#include <boost/lexical_cast.hpp>

#include "Room.hpp"
#include "IMediasoup.hpp"
#include "IWorker.hpp"
//#include "Transport/WebSocketServer.h"
#include "Config.hpp"

#include "utils/Statistics.hpp"
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include <unordered_map>
#include <mutex>

class MyWorkerObserver : public mediasoup::IWorker::Observer, public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
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
    std::shared_ptr<Room> getOrCreateRoom(const oatpp::String& roomName);
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

public:
    //Websocket-Ping all peers in the loop. Each time `interval`.
    void runPingLoop(const std::chrono::duration<v_int64, std::micro> &interval = std::chrono::minutes(1));
    //Called when socket is created
    void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;
    //Called before socket instance is destroyed.
    void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;

private:
    //webservice::server* rawWebsockServer = nullptr;
    std::string listenip;
    int listenport;
    int thread_count=1;
    //copy from canchat Lobby.hpp
    std::atomic<v_int64> m_peerIdCounter;
    std::unordered_map<oatpp::String, std::shared_ptr<Room>> m_rooms;
    std::mutex m_roomsMutex;
    //copy end
    //std::unordered_map<std::string,std::shared_ptr<Room> > rooms;
    std::vector<std::shared_ptr<mediasoup::IWorker>> workers;
    mediasoup::IMediasoup * mediasoup;
    mediasoup::WorkerSettings workerSettings;
    MyWorkerObserver myWorkerObserver;

    //std::shared_ptr<WebSocketServer> protooWebsockServer;

    Config config;
    int nextMediasoupWorkerIdx =0;
};
