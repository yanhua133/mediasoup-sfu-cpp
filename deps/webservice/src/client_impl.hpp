//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__client_impl__hpp_INCLUDED_
#define _webservice__client_impl__hpp_INCLUDED_

#include <webservice/client.hpp>
#include <webservice/executor.hpp>

#include <boost/asio/executor_work_guard.hpp>

#include <mutex>
#include <vector>
#include <thread>


namespace webservice{


	/// \brief An http and WebSocket client_impl
	class client_impl{
	public:
		/// \brief Constructor
		///
		/// \param service Handles ws sessions
		/// \param error_handler Handles error in the client_impl
		client_impl(
			class client& client,
			boost::asio::io_context& ioc,
			std::unique_ptr< class ws_handler_interface >&& service,
			std::unique_ptr< class error_handler >&& error_handler,
			std::uint8_t thread_count
		);

		client_impl(client_impl const&) = delete;

		client_impl& operator=(client_impl const&) = delete;


		/// \brief Start a new websocket connection
		void connect(
			std::string&& host,
			std::string&& port,
			std::string&& resource);


		/// \brief Reference to the error_handler
		class error_handler& error()const{
			return executor_.error();
		}

		/// \brief Reference to the WebSocket handler
		///
		/// \pre has_ws() must be true
		class ws_handler_interface& ws()const{
			return *ws_handler_;
		}


		/// \brief Get the executor object
		class executor& executor(){
			return executor_;
		}


		/// \brief Get the owning client
		class client& client(){
			return client_;
		}


	private:
		/// \brief Back reference to client object
		class client& client_;

		/// \brief Maps calles from websocket service to client
		class executor executor_;

		/// \brief Handler for WebSocket sessions
		std::unique_ptr< class ws_handler_interface > ws_handler_;

		/// \brief Keeps the io_context running
		boost::asio::executor_work_guard<
			boost::asio::io_context::executor_type > work_;
	};


}


#endif
