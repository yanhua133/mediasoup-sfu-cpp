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
#include <webservice/file_request_handler.hpp>

#include <thread>
#include <csignal>


enum class state_t{
	init,
	file_request,
	ws_open,
	ws_close,
	ws_text,
	ws_binary,
	exit
};

boost::beast::string_view to_string(state_t state)noexcept{
	switch(state){
		case state_t::init:         return "init";
		case state_t::file_request: return "file_request";
		case state_t::ws_open:      return "ws_open";
		case state_t::ws_close:     return "ws_close";
		case state_t::ws_text:      return "ws_text";
		case state_t::ws_binary:    return "ws_binary";
		case state_t::exit:         return "exit";
	}
	return "unknown state";
}

void pass(state_t expect, state_t got){
	if(expect == got){
		std::cout << "\033[1;32mpass: "
			<< to_string(got)
			<< "\033[0m\n";
	}else{
		std::cout << "\033[1;31mfail: expected "
			<< to_string(expect) << " but got " << to_string(got)
			<< "\033[0m\n";
	}
}

state_t state = state_t::init;

void check(state_t got){
	switch(state){
		case state_t::init:
			pass(state, got);
			state = state_t::file_request;
			break;
		case state_t::file_request:
			pass(state, got);
			state = state_t::ws_open;
			break;
		case state_t::ws_open:
			pass(state, got);
			state = state_t::ws_text;
			break;
		case state_t::ws_text:
			pass(state, got);
			state = state_t::ws_binary;
			break;
		case state_t::ws_binary:
			pass(state, got);
			state = state_t::ws_close;
			break;
		case state_t::ws_close:
			pass(state, got);
			state = state_t::exit;
			break;
		case state_t::exit:
			pass(state, got);
			break;
	}
}


struct request_handler
	: webservice::error_printing_request_handler<
		webservice::file_request_handler >
{
	using error_printing_request_handler::error_printing_request_handler;

	void operator()(
		webservice::http_request&& req,
		webservice::http_response&& send
	)override{
		if(req.target() != "/favicon.ico"){ // ignore favicon request
			check(state_t::file_request);
		}
		webservice::file_request_handler::operator()(
			std::move(req), std::move(send));
	}
};


struct ws_service
	: webservice::error_printing_ws_service< webservice::ws_service >
{
	void on_open(webservice::ws_identifier)override{
		check(state_t::ws_open);
	}

	void on_close(webservice::ws_identifier)override{
		check(state_t::ws_close);
	}

	void on_text(
		webservice::ws_identifier,
		std::string&& text
	)override{
		check(state_t::ws_text);
		std::cout << "text: '" << text << "'\n";
		send_text(std::move(text));
	}

	void on_binary(
		webservice::ws_identifier,
		std::vector< std::uint8_t >&& data
	)override{
		check(state_t::ws_binary);
		std::string text(data.begin(), data.end());
		std::cout << "binary: '" << text << "'\n";
		send_binary(data);
		close("shutdown");
	}
};


void signal_handler(int signum){
	std::signal(signum, SIG_DFL);
	std::cout << "\033[1;31mfail: system signal: " << signum << "\033[0m\n";
	std::raise(signum);
}


int main(){
	std::signal(SIGSEGV, &signal_handler);
	std::signal(SIGABRT, &signal_handler);
	std::signal(SIGINT, &signal_handler);

	try{
		webservice::server server(
			std::make_unique< request_handler >("browser_vs_server"),
			std::make_unique< ws_service >(),
			std::make_unique< webservice::error_printing_error_handler >(),
			boost::asio::ip::make_address("127.0.0.1"), 1234, 1);

		check(state_t::init);

		std::system("xdg-open http://127.0.0.1:1234");

		auto const start = std::chrono::system_clock::now();
		while(
			state != state_t::exit &&
			std::chrono::system_clock::now() < start + std::chrono::seconds(10)
		){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		server.shutdown();

		check(state_t::exit);

		return 0;
	}catch(std::exception const& e){
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}catch(...){
		std::cerr << "Unknown exception\n";
		return 1;
	}

}
