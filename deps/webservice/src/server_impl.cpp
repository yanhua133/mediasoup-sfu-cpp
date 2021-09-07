//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "server_impl.hpp"

#include <webservice/server.hpp>
#include <webservice/http_request_handler.hpp>
#include <webservice/ws_handler_interface.hpp>
#include <webservice/error_handler.hpp>


namespace webservice{


	server_impl::server_impl(
		class server& server,
		boost::asio::io_context& ioc,
		std::unique_ptr< http_request_handler >&& http_handler,
		std::unique_ptr< ws_handler_interface >&& ws_handler,
		std::unique_ptr< error_handler >&& error_handler,
		boost::asio::ip::address const address,
		std::uint16_t const port,
		std::uint8_t thread_count
	)
		: server_(server)
		, executor_(ioc, std::move(error_handler), [this]()noexcept{
				listener_.shutdown();
				http().shutdown();
				if(has_ws()){
					ws().shutdown();
				}
			})
		, http_handler_(std::move(http_handler))
		, ws_handler_(std::move(ws_handler))
		, listener_(*this, boost::asio::ip::tcp::endpoint{address, port}, ioc)
	{
		// Create the default http_request_handler if none exist
		if(!http_handler_){
			http_handler_ = std::make_unique< http_request_handler >();
		}

		// Make http handler ready
		http_handler_->set_server(*this);

		// Make websocket handler ready if it exists
		if(ws_handler_){
			ws_handler_->set_executor(executor_);
		}

		executor_.run(thread_count);
	}


	void server_impl::connect(
		std::string&& host,
		std::string&& port,
		std::string&& resource
	){
		if(ws_handler_){
			ws_handler_->client_connect(std::move(host), std::move(port),
				std::move(resource));
		}else{
			throw std::logic_error(
				"Called connect on a server without a websocket handler");
		}
	}


}
