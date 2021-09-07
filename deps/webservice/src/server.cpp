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


namespace webservice{


	server::server(
		std::unique_ptr< http_request_handler > http_handler,
		std::unique_ptr< ws_handler_interface > ws_handler,
		std::unique_ptr< error_handler > error_handler,
		boost::asio::ip::address const address,
		std::uint16_t const port,
		std::uint8_t const thread_count
	)
		: ioc_{thread_count}
		, impl_(std::make_unique< server_impl >(
				*this,
				ioc_,
				std::move(http_handler),
				std::move(ws_handler),
				std::move(error_handler),
				address,
				port,
				thread_count
			)) {}


	server::~server(){
		shutdown();
		block();
	}


	void server::connect(
		std::string host,
		std::string port,
		std::string resource
	){
		impl_->connect(std::move(host), std::move(port), std::move(resource));
	}


	void server::block()noexcept{
		impl_->executor().block();
	}

	void server::shutdown()noexcept{
		impl_->executor().shutdown();
	}

	bool server::is_stopped()noexcept{
		return impl_->executor().is_stopped();
	}


	boost::asio::io_context& server::get_io_context()noexcept{
		return ioc_;
	}


}
