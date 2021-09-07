//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/ws_service.hpp>
#include <webservice/client.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <csignal>


struct ws_client_service: webservice::ws_service{
	using webservice::ws_service::ws_service;

	void on_open(webservice::ws_identifier)override{
		std::cout << "open session\n";

		send_text("text from client");
	}

	void on_close(webservice::ws_identifier)override{
		std::cout << "closed\n";
	}

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		std::cout << "received text message: " << text << "\n";
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& /*data*/
	)override{
		std::cout << "received binary message\n";
	}
};


webservice::client* client = nullptr;

void on_interrupt(int signum){
	std::signal(signum, SIG_DFL);
	std::cout << "Signal: " << signum << '\n';
	client->shutdown();
	std::cout << "Signal ready\n";
}


void print_help(char const* const exec_name){
	std::cerr << "Usage: " << exec_name
		<< " <host> <port> <resource> <thread_count>\n"
		<< "Example:\n"
		<< "    " << exec_name << " host port resource\n";
}


int main(int argc, char* argv[]){
	// Check command line arguments.
	if(argc != 5){
		print_help(argv[0]);
		return 1;
	}

	try{
		auto const host = argv[1];
		auto const port = argv[2];
		auto const resource = argv[3];
		auto const thread_count = boost::numeric_cast< std::uint8_t >(
			boost::lexical_cast< unsigned >(argv[4]));

		webservice::client client(
				std::make_unique< ws_client_service >(),
				nullptr, // ignore errors and exceptions
				thread_count
			);
		client.connect(host, port, resource);

		// Allow to shutdown the client with CTRL+C
		::client = &client;
		std::signal(SIGINT, &on_interrupt);

		client.block();

		return 0;
	}catch(std::exception const& e){
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}catch(...){
		std::cerr << "Unknown exception\n";
		return 1;
	}
}
