//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__json_ws_service__hpp_INCLUDED_
#define _webservice__json_ws_service__hpp_INCLUDED_

#include "json_conversion.hpp"
#include "basic_ws_service.hpp"


namespace webservice{


	template <
		typename Value,
		typename SendBinaryType,
		typename ReceiveBinaryType = SendBinaryType >
	class basic_json_ws_service
		: public basic_ws_service< Value,
			nlohmann::json, SendBinaryType, nlohmann::json, ReceiveBinaryType >
	{
		using basic_ws_service< Value, nlohmann::json, SendBinaryType,
			nlohmann::json, ReceiveBinaryType >::basic_ws_service;
	};

	class json_ws_service
		: public basic_json_ws_service< none_t, std::vector< std::uint8_t > >
	{
		using basic_json_ws_service::basic_json_ws_service;

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
