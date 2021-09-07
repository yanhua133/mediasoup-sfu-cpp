//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "client_impl.hpp"

#include <webservice/client.hpp>
#include <webservice/ws_handler_interface.hpp>
#include <webservice/error_handler.hpp>


namespace webservice{


	client_impl::client_impl(
		class client& client,
		boost::asio::io_context& ioc,
		std::unique_ptr< ws_handler_interface >&& ws_handler,
		std::unique_ptr< error_handler >&& error_handler,
		std::uint8_t thread_count
	)
		: client_(client)
		, executor_(ioc, std::move(error_handler), [this]()noexcept{
				ws().shutdown();
				work_.reset();
			})
		, ws_handler_(std::move(ws_handler))
		, work_(boost::asio::make_work_guard(ioc))
	{
		// Make sure a websocket handler exists
		if(!ws_handler_){
			throw std::logic_error("client must have a websocket handler");
		}

		ws_handler_->set_executor(executor_);

		executor_.run(thread_count);
	}


	void client_impl::connect(
		std::string&& host,
		std::string&& port,
		std::string&& resource
	){
		ws_handler_->client_connect(std::move(host), std::move(port),
			std::move(resource));
	}


}
