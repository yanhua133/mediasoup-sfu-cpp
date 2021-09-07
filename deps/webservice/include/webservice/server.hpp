//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__server__hpp_INCLUDED_
#define _webservice__server__hpp_INCLUDED_

#include "http_request_handler.hpp"
#include "ws_handler_interface.hpp"
#include "error_handler.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>


namespace webservice{


	/// \brief An http and WebSocket server
	class server{
	public:
		/// \brief Constructor
		///
		/// \param http_handler Handles HTTP sessions
		/// \param service Handles ws sessions
		/// \param error_handler Handles error in the server
		/// \param address IP address (IPv4 or IPv6)
		/// \param port TCP Port
		/// \param thread_count Count of threads that proccess request parallel
		server(
			std::unique_ptr< http_request_handler > http_handler,
			std::unique_ptr< ws_handler_interface > service,
			std::unique_ptr< error_handler > error_handler,
			boost::asio::ip::address address,
			std::uint16_t port,
			std::uint8_t thread_count = 1
		);

		server(server const&) = delete;

		server& operator=(server const&) = delete;


		/// \brief Close all connections and wait on all processing threads
		///
		/// Calls close() and block()
		~server();


		/// \brief Start a new websocket client connection
		///
		/// A server can also connect to other websocket servers if its
		/// websocket handler supports it.
		void connect(std::string host, std::string port, std::string resource);


		/// \brief Don't accept new connections and async tasks
		///
		/// This function is not blocking. Call block() if you want to wait
		/// until all connections are closed.
		void shutdown()noexcept;

		/// \brief Wait on all processing threads
		///
		/// This effecivly blocks the current thread until the server is closed.
		void block()noexcept;

		/// \brief true after all tasks have finished
		bool is_stopped()noexcept;


		/// \brief Get reference to the internal io_context
		boost::asio::io_context& get_io_context()noexcept;


	private:
		/// \brief The io_context is required for all I/O
		boost::asio::io_context ioc_;

		/// \brief Pointer to implementation
		std::unique_ptr< class server_impl > impl_;
	};


}


#endif
