//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/error_handler.hpp>


namespace webservice{


	error_handler::~error_handler(){}

	void error_handler::on_exception(std::exception_ptr /*error*/)noexcept{}


}
