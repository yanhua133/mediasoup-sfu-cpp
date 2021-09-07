//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__http_session__hpp_INCLUDED_
#define _webservice__http_session__hpp_INCLUDED_

#include <webservice/http_response.hpp>

#include <boost/circular_buffer.hpp>

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>


namespace webservice{


	using http_request
		= boost::beast::http::request< boost::beast::http::string_body >;

	using strand
		= boost::asio::strand< boost::asio::io_context::executor_type >;


	/// \brief Handles an HTTP server session
	class http_session{
	public:
		/// \brief Take ownership of the socket and start reading
		explicit http_session(
			boost::asio::ip::tcp::socket&& socket,
			class server_impl& server
		);

		/// \brief Start timer and read
		void run();

		/// \brief Send a TCP shutdown
		void do_close();

		/// \brief Called by the HTTP handler to send a response.
		void response(std::unique_ptr< http_session_work >&& work);

		/// \brief Called when data was received
		void on_write(boost::system::error_code ec, bool close);


	private:
		/// \brief Async wait on timer
		void do_timer();

		/// \brief Async wait on read
		void do_read();

		/// \brief Stop the timer
		void stop_timer()noexcept;


		/// \brief This queue is used for HTTP pipelining.
		class queue{
			/// \brief Maximum number of responses we will queue
			static constexpr std::size_t limit = 64;

		public:
			/// \brief Constructor
			explicit queue();

			/// \brief Returns true if we have reached the queue limit
			bool is_full()const;

			/// \brief Called when a message finishes sending
			///
			/// Returns true if the caller should initiate a read
			bool on_write();

			/// \brief Called by the HTTP handler to send a response.
			void response(std::unique_ptr< http_session_work >&& work);


		private:
			/// \brief
			boost::circular_buffer< std::unique_ptr< http_session_work > >
				items_;
		};


		class server_impl& server_;
    boost::asio::io_context io_;
		boost::asio::ip::tcp::socket socket_;
		strand strand_;
		boost::asio::steady_timer timer_;
		boost::beast::flat_buffer buffer_;

		http_request req_;
		queue queue_;

		async_locker locker_;
	};


}


#endif
