//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__error_printing_error_handler__hpp_INCLUDED_
#define _webservice__error_printing_error_handler__hpp_INCLUDED_

#include <webservice/error_handler.hpp>

#include <iostream>


namespace webservice{


	struct error_printing_error_handler: error_handler{
		void on_exception(std::exception_ptr error)noexcept override{
			try{
				std::rethrow_exception(error);
			}catch(std::exception const& e){
				std::cout << "\033[1;31mfail error_handler: unexpected "
					"exception: " << e.what() << "\033[0m\n";
			}catch(...){
				std::cout << "\033[1;31mfail error_handler: unexpected "
					"unknown exception\033[0m\n";
			}
		}
	};


}


#endif
