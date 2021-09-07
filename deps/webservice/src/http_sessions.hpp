//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__http_sessions__hpp_INCLUDED_
#define _webservice__http_sessions__hpp_INCLUDED_

#include <webservice/async_locker.hpp>

#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>

#include <set>
#include <memory>


namespace webservice{


	using ws_stream
		= boost::beast::websocket::stream< boost::asio::ip::tcp::socket >;

	using strand
		= boost::asio::strand< boost::asio::io_context::executor_type >;

	using http_request
		= boost::beast::http::request< boost::beast::http::string_body >;


	class http_session;
	class http_request_handler;


	class http_sessions{
		struct less{
			using is_transparent = void;

			bool operator()(
				std::unique_ptr< http_session > const& l,
				std::unique_ptr< http_session > const& r
			)const noexcept{
				return l.get() < r.get();
			}

			bool operator()(
				http_session* l,
				std::unique_ptr< http_session > const& r
			)const noexcept{
				return l < r.get();
			}

			bool operator()(
				std::unique_ptr< http_session > const& l,
				http_session* r
			)const noexcept{
				return l.get() < r;
			}
		};


	public:
		using set = std::set< std::unique_ptr< http_session >, less >;


		http_sessions(class server_impl& server);

		http_sessions(http_sessions const&) = delete;


		http_sessions& operator=(http_sessions const&) = delete;


		class server& server()const noexcept;


		void async_emplace(boost::asio::ip::tcp::socket&& socket)noexcept;

		void async_erase(http_session* session)noexcept;


		void shutdown()noexcept;

		/// \brief true if server is shutting down
		bool is_shutdown()noexcept;


	private:
		class server_impl& server_;
		async_locker locker_;
		async_locker::lock run_lock_;
		async_locker::lock shutdown_lock_;
		strand strand_;
		set set_;
	};


}


#endif
