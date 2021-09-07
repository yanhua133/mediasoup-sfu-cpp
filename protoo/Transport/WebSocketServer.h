//
// Created by 潘晨光 on 2021/2/3.
//

#ifndef TESTBEASTCLION_WEBSOCKETSERVER_H
#define TESTBEASTCLION_WEBSOCKETSERVER_H

#include "HttpTransport.h"
#include "WebSocketTransport.h"
#include <boost/asio/signal_set.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <vector>
#include "../utils/EventEmitter.hpp"


class WebSocketServer: public EventEmitter,public std::enable_shared_from_this<WebSocketServer> {
public:
    WebSocketServer(const char* address, int port, const char* doc_root);
    ~WebSocketServer();
    void runWebSocketServer();
private:
    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec, tcp::socket socket);
    
private:
    // The io_context is required for all I/O
    net::io_context m_ioc{5};
    std::vector<std::thread> m_vecThread;
    tcp::acceptor acceptor_;
    const char* m_ipAddress;
    int m_port;
    const char* m_docRoot;
};


#endif //TESTBEASTCLION_WEBSOCKETSERVER_H
