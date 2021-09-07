//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_session__hpp_INCLUDED_
#define _webservice__ws_session__hpp_INCLUDED_

#include "async_locker.hpp"
#include "shared_const_buffer.hpp"

#include <boost/beast/websocket.hpp>

#include <boost/asio/strand.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>

#include <memory>
#include <chrono>


namespace webservice{


	using http_request
		= boost::beast::http::request< boost::beast::http::string_body >;

	using ws_stream
		= boost::beast::websocket::stream< boost::asio::ip::tcp::socket >;

	using strand
		= boost::asio::strand< boost::asio::io_context::executor_type >;


	class ws_service_interface;


	/// \brief Base of WebSocket sessions
	class ws_session{
	public:
		/// \brief Take ownership of the socket
		explicit ws_session(
			ws_stream&& ws,
			ws_service_interface& service,
			std::chrono::milliseconds ping_time);

		ws_session(ws_session const&) = delete;


		/// \brief Destructor
		~ws_session();


		ws_session& operator=(ws_session const&) = delete;


		/// \brief Start the asynchronous operation for server sessions
		void do_accept(http_request&& req);


		/// \brief Start the asynchronous operation for client sessions
		void start();


		/// \brief Send a message
		void send(bool is_text, shared_const_buffer buffer)noexcept;

		/// \brief Close the session
		void close(boost::beast::websocket::close_reason reason)noexcept;


	private:
		/// \brief Async wait on timer
		///
		/// The timer is restarted after any received message.
		///
		/// Send a ping after the first timeout. If it timeouts a second time
		/// after that, close the session.
		void do_timer();

		/// \brief Called to indicate activity from the remote peer
		void activity();

		/// \brief Read another message
		void do_read();

		/// \brief Called when a message was written
		void on_write(boost::system::error_code ec);

		/// \brief Set timers expires_after
		void restart_timer();

		/// \brief Close session on socket level
		void close_socket()noexcept;

		/// \brief Stop the timer
		void stop_timer()noexcept;


		/// \brief Send the next outstanding message or close
		void do_write();

		/// \brief Initiate the first timer call
		void start_timer();


		/// \brief Called with when a sessions starts
		void on_open()noexcept;

		/// \brief Called with when a sessions ends
		void on_close()noexcept;

		/// \brief Called when a text message
		void on_text(boost::beast::multi_buffer&& buffer)noexcept;

		/// \brief Called when a binary message
		void on_binary(boost::beast::multi_buffer&& buffer)noexcept;

		/// \brief Called when an error occured
		void on_error(
			boost::beast::string_view location,
			boost::system::error_code ec)noexcept;

		/// \brief Called when an exception was thrown
		void on_exception(std::exception_ptr error)noexcept;


		/// \brief Write queue entry type
		struct write_data{
			bool is_text;
			shared_const_buffer data;
		};


		/// \brief Reference to the owning service
		ws_service_interface& service_;

		/// \brief The websocket stream
		ws_stream ws_;

		/// \brief Serialized operations on websocket
		strand strand_;

		/// \brief Serialized call of the handlers
		strand handler_strand_;

		/// \brief Send ping after timeout, close session after second timeout
		boost::asio::steady_timer timer_;

		/// \brief Protectes async operations
		async_locker locker_;

		/// \brief Write queue
		boost::circular_buffer< write_data > write_list_;

		/// \brief Optional close reason
		std::unique_ptr< boost::beast::websocket::close_reason > close_reason_;

		/// \brief Timer timeout time
		std::chrono::milliseconds const ping_time_;

		/// \brief Read buffer
		boost::beast::multi_buffer buffer_;

		/// \brief Ping counter
		std::size_t ping_counter_{0};

		/// \brief Ping flag
		bool wait_on_pong_{false};

		/// \brief true after is_open() call
		bool is_open_{false};
	};


}


#endif
