//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/ws_service_interface.hpp>


namespace webservice{


	void ws_service_interface::on_open(ws_identifier /*identifier*/){}

	void ws_service_interface::on_close(ws_identifier /*identifier*/){}

	void ws_service_interface::on_text(
		ws_identifier /*identifier*/,
		boost::beast::multi_buffer&& /*buffer*/){}

	void ws_service_interface::on_binary(
		ws_identifier /*identifier*/,
		boost::beast::multi_buffer&& /*buffer*/){}

	void ws_service_interface::on_exception(
		ws_identifier /*identifier*/,
		std::exception_ptr /*error*/)noexcept{}


	void ws_service_interface::on_erase(ws_identifier /*identifier*/)noexcept{}


}
