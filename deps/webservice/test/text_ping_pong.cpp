//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "error_printing_ws_service.hpp"
#include "error_printing_error_handler.hpp"
#include "error_printing_request_handler.hpp"

#include <webservice/server.hpp>
#include <webservice/ws_service.hpp>
#include <webservice/client.hpp>

#include <boost/lexical_cast.hpp>

#include <thread>
#include <csignal>


struct request_handler
	: webservice::error_printing_request_handler<
		webservice::http_request_handler >
{
	using error_printing_request_handler::error_printing_request_handler;

	void operator()(
		webservice::http_request&& req,
		webservice::http_response&& send
	)override{
		std::cout << "\033[1;31mfail: unexpected file request '"
			<< req.target() << "'\033[0m\n";
		webservice::http_request_handler::operator()(
			std::move(req), std::move(send));
	}
};


struct ws_service
	: webservice::error_printing_ws_service< webservice::ws_service >
{
	int count = 0;

	void on_open(webservice::ws_identifier)override{
		send_text("0");
	}

	void on_close(webservice::ws_identifier)override{
		executor().shutdown();
	}

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		int received_count = boost::lexical_cast< int >(text);
		if(received_count == count){
			if(count % 1000 == 0){
				std::cout << "\033[1;32mserver pass: '"
					<< text << "'\033[0m\n";
			}
			++count;

			if(count > 1000000){
				close("shutdown");
				return;
			}

			send_text(std::to_string(count));
		}else{
			std::cout << "\033[1;31mfail: server expected '"
				<< std::to_string(count) << "' but got '" << text
				<< "'\033[0m\n";
			close("shutdown");
		}
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& data
	)override{
		std::string text(data.begin(), data.end());
		std::cout << "\033[1;31mfail: server unexpected binary message '"
			<< text << "'\033[0m\n";
		close("shutdown");
	}
};


struct ws_client_service
	: webservice::error_printing_ws_service< webservice::ws_service >
{
	int count = 0;

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		int received_count = boost::lexical_cast< int >(text);
		if(received_count == count){
			if(count % 1000 == 0){
				std::cout << "\033[1;32mclient pass: '"
					<< text << "'\033[0m\n";
			}
			++count;
			send_text(text);
		}else{
			std::cout << "\033[1;31mfail: client expected '"
				<< std::to_string(count) << "' but got '" << text
				<< "'\033[0m\n";
			close("shutdown");
		}
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& data
	)override{
		std::string text(data.begin(), data.end());
		std::cout << "\033[1;31mfail: client unexpected binary message '"
			<< text << "'\033[0m\n";
		close("shutdown");
	}
};


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
		{
			using std::make_unique;
			webservice::server server(
				make_unique< request_handler >(),
				make_unique< ws_service >(),
				make_unique< webservice::error_printing_error_handler >(),
				boost::asio::ip::make_address("127.0.0.1"), 1234, 1);

			webservice::client client(
				std::make_unique< ws_client_service >(),
				std::make_unique< webservice::error_handler >());
			client.connect("127.0.0.1", "1234", "/");

			server.block();
		}

		return 0;
	}catch(std::exception const& e){
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}catch(...){
		std::cerr << "Unknown exception\n";
		return 1;
	}

}
