//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#ifndef BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_WEBSOCKET_SESSION_HPP
#define BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_WEBSOCKET_SESSION_HPP

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include "../utils/EventEmitter.hpp"
#include "../Message.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

/** Represents an active WebSocket connection to the server
*/
class WebSocketTransport : public boost::enable_shared_from_this<WebSocketTransport>, public EventEmitter
{
    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    std::vector<boost::shared_ptr<std::string const>> queue_;

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    WebSocketTransport(tcp::socket&& socket);
    ~WebSocketTransport();
    bool closed();
    void close();

    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);

    // Send a message
    void send(boost::shared_ptr<std::string const> const& ss);
    void send(json ss);

private:
    void on_send(boost::shared_ptr<std::string const> const& ss);
private:
    bool m_closed = false;
};

template<class Body, class Allocator>
void WebSocketTransport::run(http::request<Body, http::basic_fields<Allocator>> req)
{
    //std::cout<< "roomId:" << static_cast<std::string>(req[boost::beast::http::field::host]) << "peerId:" <<std::endl;
    // Set suggested timeout settings for the websocket
    ws_.set_option(
            websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::sec_websocket_protocol,
                        "protoo");
            }));

    // Accept the websocket handshake
    ws_.async_accept(
            req,
            beast::bind_front_handler(
                    &WebSocketTransport::on_accept,
                    shared_from_this()));
}

#endif
