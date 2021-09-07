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


namespace webservice{


	client::client(
		std::unique_ptr< ws_handler_interface > ws_handler,
		std::unique_ptr< error_handler > error_handler,
		std::uint8_t const thread_count
	)
		: ioc_{thread_count}
		, impl_(std::make_unique< client_impl >(
				*this,
				ioc_,
				std::move(ws_handler),
				std::move(error_handler),
				thread_count
			)) {}


	client::~client(){
		shutdown();
		block();
	}


	void client::connect(
		std::string host,
		std::string port,
		std::string resource
	){
		impl_->connect(std::move(host), std::move(port), std::move(resource));
	}


	void client::block()noexcept{
		impl_->executor().block();
	}

	void client::shutdown()noexcept{
		impl_->executor().shutdown();
	}

	bool client::is_stopped()noexcept{
		return impl_->executor().is_stopped();
	}


	boost::asio::io_context& client::get_io_context()noexcept{
		return ioc_;
	}


}
