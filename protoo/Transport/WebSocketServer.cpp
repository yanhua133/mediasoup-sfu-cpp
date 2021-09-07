//
// Created by 潘晨光 on 2021/2/3.
//

#include "WebSocketServer.h"
#include "../utils/Log.hpp"
WebSocketServer::WebSocketServer(const char* address, int port, const char* doc_root): acceptor_(m_ioc)
{
   
    auto const threads = 5;
    m_ipAddress = address;
    m_port = port;
    m_docRoot = doc_root;
    // TODO how to set options to server (keepalive:true keepaliveInterval:60000)
    // Create and launch a listening port
    //initListener(tcp::endpoint{ip_address, ip_port}, doc_root);
}

WebSocketServer::~WebSocketServer()
{
    m_ioc.stop();
}

void WebSocketServer::runWebSocketServer()
{
    auto const threads = 5;
    auto ip_address = net::ip::make_address(m_ipAddress);
    auto ip_port = static_cast<unsigned short>(m_port);
    tcp::endpoint endpoint=tcp::endpoint{ip_address, ip_port};
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }
    std::cout<< "[WebSocketServer] ws server is running and listening for remote connections!!!"<<std::endl;
    // Start listening for connections
    acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }

    // The new connection gets its own strand
    acceptor_.async_accept(
            net::make_strand(m_ioc),
            beast::bind_front_handler(
                    &WebSocketServer::on_accept,
                    shared_from_this()));
    
    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(m_ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](boost::system::error_code const&, int)
            {
                // Stop the io_context. This will cause run()
                // to return immediately, eventually destroying the
                // io_context and any remaining handlers in it.
                m_ioc.stop();
            });

    // Run the I/O service on the requested number of threads
    m_vecThread.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        m_vecThread.emplace_back(
                [&]
                {
                    m_ioc.run();
                });
    m_ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)
    // Block until all the threads exit
    for(auto& t : m_vecThread)
        t.join();
}

// Report a failure
void WebSocketServer::fail(beast::error_code ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
    MS_lOGE("WebSocketServer fail connection err =%s",ec.message().c_str());
}

// Handle a connection
void WebSocketServer::on_accept(beast::error_code ec, tcp::socket socket)
{
    if(ec)
        return fail(ec, "accept");
    else
    {
        // Launch a new session for this connection
        boost::shared_ptr<HttpTransport> httpSession = boost::make_shared<HttpTransport>(
                std::move(socket),m_docRoot);
        httpSession->run();
        httpSession->on("connectionrequest",[&](std::string&& roomid, std::string&& peerid, WebSocketTransport* transport){
                this->emit("connectionrequest",roomid, peerid, transport);
        });
        //std::cout<<"Handle a connection"<<std::endl;
        MS_lOGD("on_accept Handle a new connection ");
        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(m_ioc),
                beast::bind_front_handler(
                        &WebSocketServer::on_accept,
                        shared_from_this()));
    }
}
