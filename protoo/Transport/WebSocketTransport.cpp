//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "WebSocketTransport.h"
#include <iostream>
#include "../utils/Log.hpp"

WebSocketTransport::WebSocketTransport(
        tcp::socket&& socket)
        : ws_(std::move(socket))
{
}

WebSocketTransport::~WebSocketTransport()
{
    // Remove this session from the list of active sessions
    //state_->leave(this);
}

bool WebSocketTransport::closed()
{

    return m_closed;
}

void WebSocketTransport::close()
{
    if(m_closed){
        return;
    }
    m_closed = true;
    ws_.close("just close this ws");
    MS_lOGD("WebSocketTransport::close  ");
    this->emit("close");
}

void WebSocketTransport::fail(beast::error_code ec, char const* what)
{
    // Don't report these
    if (ec == net::error::operation_aborted ||
        ec == websocket::error::closed) {
        this->emit("close"); 
        return;
    }

    std::cerr << what << ": " << ec.message() << "\n";
    MS_lOGE("WebSocketTransport fail connection err =%s",ec.message().c_str());
    this->emit("close"); 
}

void WebSocketTransport::on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    //state_->join(this);

    // Read a message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &WebSocketTransport::on_read,
                    shared_from_this()));
}

void WebSocketTransport::on_read(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");

    // Send to all connections
    //state_->send(beast::buffers_to_string(buffer_.data()));
    std::string message = beast::buffers_to_string(buffer_.data());
    //TODO emit received message to uplevel
    if(message != "ping")
    {
        this->emit("message", protoo::Message::parse(message) );
    }
   
    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &WebSocketTransport::on_read,
                    shared_from_this()));
}

void WebSocketTransport::send(boost::shared_ptr<std::string const> const& ss)
{
    if (m_closed) {
        return;
    }
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.
    net::post(
            ws_.get_executor(),
            beast::bind_front_handler(
                    &WebSocketTransport::on_send,
                    shared_from_this(),
                    ss));
}

void WebSocketTransport::send(json ss)
{
    if (m_closed) { 
        return;
    }
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.
    net::post(
            ws_.get_executor(),
            beast::bind_front_handler(
                    &WebSocketTransport::on_send,
                    shared_from_this(),
                    boost::make_shared<std::string const>(ss.dump())));
}

void WebSocketTransport::on_send(boost::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
            net::buffer(*queue_.front()),
            beast::bind_front_handler(
                    &WebSocketTransport::on_write,
                    shared_from_this()));
}

void WebSocketTransport::on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
                net::buffer(*queue_.front()),
                beast::bind_front_handler(
                        &WebSocketTransport::on_write,
                        shared_from_this()));
}
