//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__client__hpp_INCLUDED_
#define _webservice__client__hpp_INCLUDED_

#include "ws_handler_interface.hpp"
#include "error_handler.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>


namespace webservice{


	/// \brief An http and WebSocket client
	class client{
	public:
		/// \brief Constructor
		///
		/// \param service Handles ws sessions
		/// \param error_handler Handles error in the client
		/// \param address IP address (IPv4 or IPv6)
		/// \param port TCP Port
		/// \param thread_count Count of threads that proccess request parallel
		client(
			std::unique_ptr< ws_handler_interface > service,
			std::unique_ptr< error_handler > error_handler,
			std::uint8_t thread_count = 1
		);

		client(client const&) = delete;

		client& operator=(client const&) = delete;


		/// \brief Close all connections and wait on all processing threads
		///
		/// Calls close() and block()
		~client();


		/// \brief Start a new websocket connection
		void connect(std::string host, std::string port, std::string resource);


		/// \brief Don't accept new connections and async tasks
		///
		/// This function is not blocking. Call block() if you want to wait
		/// until all connections are closed.
		void shutdown()noexcept;

		/// \brief Wait on all processing threads
		///
		/// This effecivly blocks the current thread until the client is closed.
		void block()noexcept;

		/// \brief true after all tasks have finished
		bool is_stopped()noexcept;


		/// \brief Get reference to the internal io_context
		boost::asio::io_context& get_io_context()noexcept;


	private:
		/// \brief The io_context is required for all I/O
		boost::asio::io_context ioc_;

		/// \brief Pointer to implementation
		std::unique_ptr< class client_impl > impl_;
	};


}


#endif
