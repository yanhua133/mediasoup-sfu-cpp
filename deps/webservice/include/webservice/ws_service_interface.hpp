//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_service_interface__hpp_INCLUDED_
#define _webservice__ws_service_interface__hpp_INCLUDED_

#include "ws_handler_interface.hpp"
#include "ws_identifier.hpp"

#include <boost/beast/core/multi_buffer.hpp>

#include <exception>


namespace webservice{


	/// \brief The virtual callback functions for server websocket sessions
	class ws_service_interface: public ws_handler_interface{
	public:
		/// \brief Destructor
		virtual ~ws_service_interface() = default;


		/// \brief Called when a sessions starts
		///
		/// Default implementation does nothing.
		virtual void on_open(ws_identifier identifier);

		/// \brief Called when a sessions ends
		///
		/// Default implementation does nothing.
		virtual void on_close(ws_identifier identifier);

		/// \brief Called when a session received a text message
		///
		/// Default implementation does nothing.
		virtual void on_text(
			ws_identifier identifier,
			boost::beast::multi_buffer&& buffer);

		/// \brief Called when a session received a binary message
		///
		/// Default implementation does nothing.
		virtual void on_binary(
			ws_identifier identifier,
			boost::beast::multi_buffer&& buffer);

		/// \brief Called when an exception was thrown
		///
		/// Default implementation does nothing.
		virtual void on_exception(
			ws_identifier identifier,
			std::exception_ptr error)noexcept;

		using ws_handler_interface::on_exception;


		/// \brief Called when a sessions is ready to be erased
		///
		/// Default implementation does nothing.
		virtual void on_erase(ws_identifier identifier)noexcept;
	};


}


#endif
