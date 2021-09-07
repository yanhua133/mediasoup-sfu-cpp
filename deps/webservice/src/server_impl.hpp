//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__server_impl__hpp_INCLUDED_
#define _webservice__server_impl__hpp_INCLUDED_

#include "listener.hpp"

#include <webservice/server.hpp>
#include <webservice/executor.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/ip/address.hpp>

#include <mutex>
#include <vector>
#include <thread>


namespace webservice{


	/// \brief An http and WebSocket server_impl
	class server_impl{
	public:
		/// \brief Constructor
		///
		/// \param http_handler Handles HTTP sessions
		/// \param service Handles ws sessions
		/// \param error_handler Handles error in the server_impl
		/// \param address IP address (IPv4 or IPv6)
		/// \param port TCP Port
		server_impl(
			class server& server,
			boost::asio::io_context& ioc,
			std::unique_ptr< class http_request_handler >&& http_handler,
			std::unique_ptr< class ws_handler_interface >&& service,
			std::unique_ptr< class error_handler >&& error_handler,
			boost::asio::ip::address address,
			std::uint16_t port,
			std::uint8_t thread_count
		);

		server_impl(server_impl const&) = delete;

		server_impl& operator=(server_impl const&) = delete;


		/// \brief Start a new websocket client connection
		void connect(
			std::string&& host,
			std::string&& port,
			std::string&& resource);


		/// \brief true if a WebSocket handler is set, false otherwise
		bool has_ws()const{
			return ws_handler_.get() != nullptr;
		}


		/// \brief Reference to the error_handler
		class error_handler& error()const{
			return executor_.error();
		}

		/// \brief Reference to the http handler
		class http_request_handler& http()const{
			return *http_handler_;
		}

		/// \brief Reference to the WebSocket handler
		///
		/// \pre has_ws() must be true
		class ws_handler_interface& ws()const{
			assert(ws_handler_.get() != nullptr);
			return *ws_handler_;
		}


		/// \brief Get the executor object
		class executor& executor(){
			return executor_;
		}


		/// \brief Get the owning server
		class server& server(){
			return server_;
		}


	private:
		/// \brief Back reference to the server
		class server& server_;

		/// \brief Maps calles from websocket service to server
		class executor executor_;

		/// \brief Handler for HTTP sessions
		std::unique_ptr< class http_request_handler > http_handler_;

		/// \brief Handler for WebSocket sessions
		std::unique_ptr< class ws_handler_interface > ws_handler_;

		/// \brief Accepts incoming connections and launches the sessions
		listener listener_;
	};


}


#endif
