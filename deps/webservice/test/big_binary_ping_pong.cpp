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
#include <webservice/client.hpp>
#include <webservice/ws_service.hpp>

#include <boost/lexical_cast.hpp>

#include <iomanip>
#include <thread>
#include <random>
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


std::vector< std::uint8_t > binary_data;

void fill_data(){
	constexpr std::size_t size = 1024*1024*16;
	std::cout << "begin fill data vector\n";
	binary_data.clear();
	binary_data.reserve(size);
    std::random_device rd;
    std::uniform_int_distribution< std::uint8_t > dist;
	std::cout << "====================\n";
	for(std::size_t i = 0; i < size; ++i){
		binary_data.push_back(dist(rd));
		if(i % ((size / 20) + 1) == 0){
			std::cout << "-" << std::flush;
		}
	}
	std::cout << "\nend fill data vector" << std::endl;
}

struct ws_server_service
	: webservice::error_printing_ws_service< webservice::ws_service >
{
	std::size_t count = 0;

	void on_open(webservice::ws_identifier)override{
		std::thread([this]{
				fill_data();
				send_binary(binary_data);
			}).detach();
	}

	void on_close(webservice::ws_identifier)override{
		executor().shutdown();
	}

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		std::cout << "\033[1;31mfail: server unexpected text message '"
			<< text << "'\033[0m\n";
		close("shutdown");
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& data
	)override{
		if(data == binary_data){
			std::cout << "\033[1;32mserver pass vector with "
				<< data.size() << "\033[0m\n";
			if(count < 10){
				++count;
				std::thread([this]{
						fill_data();
						send_binary(binary_data);
					}).detach();
			}else{
				close("shutdown");
			}
		}else{
			std::cout << "\033[1;31mfail: server expected vector with size "
				<< binary_data.size() << " but got " << data.size()
				<< " with different data\033[0m\n";
			if(binary_data.size() == data.size()){
				std::cout << "     pos: expected;      got\n";
				auto i1 = binary_data.begin();
				auto i2 = data.begin();
				for(
					std::size_t pos = 0, printed = 0;
					printed < 20 && i1 != binary_data.end();
					++pos, ++i1, ++i2
				){
					if(printed > 0 || *i1 != *i2){
						++printed;
						std::cout
							<< std::setw(8) << pos << ": "
							<< std::setw(8) << static_cast< int >(*i1) << "; "
							<< std::setw(8) << static_cast< int >(*i2) << "\n";
					}
				}
			}
			close("shutdown");
		}
	}
};


struct ws_client_service
	: webservice::error_printing_ws_service< webservice::ws_service >
{
	using error_printing_ws_service::error_printing_ws_service;

	int count = 0;

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		std::cout << "\033[1;31mfail: client unexpected text message '"
			<< text << "'\033[0m\n";
		close("shutdown");
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& data
	)override{
		if(data == binary_data){
			std::cout << "\033[1;32mclient pass vector with "
				<< data.size() << "\033[0m\n";
			send_binary(std::move(data));
		}else{
			std::cout << "\033[1;31mfail: client expected vector with size "
				<< binary_data.size() << " but got " << data.size()
				<< " with different data\033[0m\n";
			if(binary_data.size() == data.size()){
				std::cout << "     pos: expected;      got\n";
				auto i1 = binary_data.begin();
				auto i2 = data.begin();
				for(
					std::size_t pos = 0, printed = 0;
					printed < 20 && i1 != binary_data.end();
					++pos, ++i1, ++i2
				){
					if(printed > 0 || *i1 != *i2){
						++printed;
						std::cout
							<< std::setw(8) << pos << ": "
							<< std::setw(8) << static_cast< int >(*i1) << "; "
							<< std::setw(8) << static_cast< int >(*i2) << "\n";
					}
				}
			}
			close("shutdown");
		}
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
		using std::make_unique;

		auto ws_server_service = make_unique< struct ws_server_service >();
		ws_server_service->set_ping_time(std::chrono::milliseconds(4000));

		webservice::server server(
			make_unique< request_handler >(),
			std::move(ws_server_service),
			make_unique< webservice::error_printing_error_handler >(),
			boost::asio::ip::make_address("127.0.0.1"), 1234, 1);


		auto ws_client_service = make_unique< struct ws_client_service >();
		ws_client_service->set_ping_time(std::chrono::milliseconds(4000));

		webservice::client client(
			std::move(ws_client_service),
			make_unique< webservice::error_printing_error_handler >());

		client.connect("127.0.0.1", "1234", "/");

		server.block();

		return 0;
	}catch(std::exception const& e){
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}catch(...){
		std::cerr << "Unknown exception\n";
		return 1;
	}

}
