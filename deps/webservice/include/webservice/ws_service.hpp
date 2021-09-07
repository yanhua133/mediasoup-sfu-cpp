//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_service__hpp_INCLUDED_
#define _webservice__ws_service__hpp_INCLUDED_

#include "basic_ws_service.hpp"


namespace webservice{


	/// \brief A WebSocket service without data and with std::string for text
	///        and std::vector< std::uint8_t > for binary messages
	class ws_service
		: public basic_ws_service<
			none_t, std::string, std::vector< std::uint8_t > >
	{
		using basic_ws_service::basic_ws_service;

		/// \brief Create a new ws_session
		void on_server_connect(
			boost::asio::ip::tcp::socket&& socket,
			http_request&& req
		){
			async_server_connect(std::move(socket), std::move(req));
		}

		/// \brief Create a new client websocket session
		void on_client_connect(
			std::string&& host,
			std::string&& port,
			std::string&& resource
		){
			async_client_connect(std::move(host), std::move(port),
				std::move(resource));
		}
	};


}


#endif
