//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/server.hpp>
#include <webservice/ws_service.hpp>
#include <webservice/client.hpp>

#include <csignal>
#include <iostream>


void signal_handler(int signum){
	std::signal(signum, SIG_DFL);
	std::cout << "Signal: " << signum << '\n';
	std::raise(signum);
}


int main(){
	std::signal(SIGSEGV, &signal_handler);
	std::signal(SIGABRT, &signal_handler);
	std::signal(SIGINT, &signal_handler);

	try{
		webservice::server server(
			std::make_unique< webservice::http_request_handler >(),
			std::make_unique< webservice::ws_service >(),
			std::make_unique< webservice::error_handler >(),
			boost::asio::ip::make_address("127.0.0.1"), 1234, 1);

		webservice::client client(
			std::make_unique< webservice::ws_service >(),
			std::make_unique< webservice::error_handler >());
		client.connect("127.0.0.1", "1234", "/");

		return 0;
	}catch(std::exception const& e){
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}catch(...){
		std::cerr << "Unknown exception\n";
		return 1;
	}

}
