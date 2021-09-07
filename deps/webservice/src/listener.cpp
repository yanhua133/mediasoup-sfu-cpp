//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "listener.hpp"
#include "server_impl.hpp"

#include <webservice/error_handler.hpp>
#include <webservice/http_request_handler.hpp>


namespace webservice{


	listener::listener(
		server_impl& server,
		boost::asio::ip::tcp::endpoint endpoint,
		boost::asio::io_context& ioc
	)
		: server_(server)
		, acceptor_(ioc)
		, socket_(ioc)
	{
		// Open the acceptor
		acceptor_.open(endpoint.protocol());

		// Allow port reuse
		using acceptor = boost::asio::ip::tcp::acceptor;
		acceptor_.set_option(acceptor::reuse_address(true));

		// Bind to the server address
		acceptor_.bind(endpoint);

		// Start listening for connections
		acceptor_.listen(boost::asio::socket_base::max_listen_connections);

		// Start accepting incoming connections
		do_accept();
	}


	void listener::do_accept(){
		acceptor_.async_accept(
			socket_,
			[this](boost::system::error_code ec){
				if(ec == boost::asio::error::operation_aborted){
					return;
				}

				if(ec){
					server_.error().on_exception(
						std::make_exception_ptr(boost::system::system_error(
							ec, "server listener accept")));
					return;
				}else{
					// Create and run the http_session
					server_.http().async_emplace(std::move(socket_));
				}

				// Accept another connection
				do_accept();
			});
	}

	void listener::shutdown()noexcept{
		// Don't accept new sessions
		acceptor_.close();
	}


}
