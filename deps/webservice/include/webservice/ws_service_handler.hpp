//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_service_handler__hpp_INCLUDED_
#define _webservice__ws_service_handler__hpp_INCLUDED_

#include "ws_handler_interface.hpp"
#include "ws_service_interface.hpp"

#include <memory>
#include <set>


namespace webservice{


	/// \brief Refers sessions to sub-service by the requested target name
	class ws_service_handler: public ws_handler_interface{
	public:
		/// \brief Constructor
		ws_service_handler();

		/// \brief Destructor
		///
		/// Block until last async operation has finished.
		~ws_service_handler()override;


		/// \brief Add ws_service_interface that is used for sessions with
		///        resource name
		///
		/// \throw std::logic_error if a ws_service_interface with same name
		///                         did already exist
		///
		/// Thread safe: Yes.
		void add_service(
			std::string name,
			std::unique_ptr< class ws_service_interface > service)noexcept;

		/// \brief Erase ws_service_interface with name
		///
		/// \attention A service must not remove itself!
		///
		/// \throw std::logic_error if no ws_service_interface with name did
		///                         exist
		///
		/// Thread safe: Yes.
		void erase_service(std::string name)noexcept;


	private:
		/// \brief Create the service map
		void on_executor()final;

		/// \brief Create a new websocket server session
		void on_server_connect(
			boost::asio::ip::tcp::socket&& socket,
			http_request&& req)noexcept final;

		/// \brief Create a new client websocket session
		void on_client_connect(
			std::string&& host,
			std::string&& port,
			std::string&& resource)noexcept final;

		/// \brief Call shutdown on all services
		void on_shutdown()noexcept final;


		/// \brief true after on_shutdown async has finished
		///
		/// \attention This is not equivalent with is_shutdown().
		bool shutdown_{false};

		/// \brief Pointer to implementation
		std::unique_ptr< struct ws_service_handler_impl > impl_;
	};


}


#endif
