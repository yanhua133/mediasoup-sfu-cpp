//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__error_printing_request_handler__hpp_INCLUDED_
#define _webservice__error_printing_request_handler__hpp_INCLUDED_

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <exception>
#include <iostream>
#include <string>


namespace webservice{


	template < typename Base >
	struct error_printing_request_handler: Base{
		using Base::Base;

		void on_exception(std::exception_ptr error)noexcept override{
			try{
				std::rethrow_exception(error);
			}catch(std::exception const& e){
				std::cout << "\033[1;31mfail http_request_handler: unexpected "
					"exception: " << e.what() << "\033[0m\n";
			}catch(...){
				std::cout << "\033[1;31mfail http_request_handler: unexpected "
					"unknown exception\033[0m\n";
			}
		}
	};


}


#endif
