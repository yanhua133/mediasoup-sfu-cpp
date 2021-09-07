//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/server.hpp>
#include <webservice/client.hpp>
#include <webservice/ws_service.hpp>

#include <gtest/gtest.h>

#include <boost/type_index.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>


using namespace webservice;


struct request_handler: webservice::http_request_handler{
	void on_exception(std::exception_ptr error)noexcept override{
		try{
			std::rethrow_exception(error);
		}catch(std::exception const& e){
			FAIL() << "unexpected exception: ["
				<< boost::typeindex::type_id_runtime(e).pretty_name() << "] "
				<< e.what();
		}catch(...){
			FAIL() << "unknown exception";
		}
	}
};

struct ws_service: webservice::ws_service{
	void on_exception(std::exception_ptr error)noexcept override{
		try{
			std::rethrow_exception(error);
		}catch(std::exception const& e){
			FAIL() << "unexpected exception: ["
				<< boost::typeindex::type_id_runtime(e).pretty_name() << "] "
				<< e.what();
		}catch(...){
			FAIL() << "unknown exception";
		}
	}

	void on_exception(ws_identifier, std::exception_ptr error)noexcept override{
		try{
			std::rethrow_exception(error);
		}catch(std::exception const& e){
			FAIL() << "unexpected exception: ["
				<< boost::typeindex::type_id_runtime(e).pretty_name() << "] "
				<< e.what();
		}catch(...){
			FAIL() << "unknown exception";
		}
	}
};

struct error_handler: webservice::error_handler{
	void on_exception(std::exception_ptr error)noexcept override{
		try{
			std::rethrow_exception(error);
		}catch(std::exception const& e){
			FAIL() << "unexpected exception: ["
				<< boost::typeindex::type_id_runtime(e).pretty_name() << "] "
				<< e.what();
		}catch(...){
			FAIL() << "unknown exception";
		}
	}
};


using stream = boost::beast::websocket::stream< boost::asio::ip::tcp::socket >;


std::string const host = "127.0.0.1";
std::uint16_t const port = 1234;

stream connected_client(boost::asio::io_context& ioc){
	boost::asio::ip::tcp::resolver resolver{ioc};
	stream ws{ioc};

	auto const results = resolver.resolve(host, std::to_string(port));

	boost::asio::connect(ws.next_layer(), results.begin(), results.end());

	ws.handshake(host, "/");
	return ws;
}

void read(stream& ws){
	boost::beast::multi_buffer buffer;
	boost::system::error_code ec;
	ws.read(buffer, ec);
}

void wait(server& s){
	auto end = std::chrono::high_resolution_clock::now()
		+ std::chrono::seconds(1);
	while(std::chrono::high_resolution_clock::now() < end){
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if(s.is_stopped()){
			return;
		}
	}

	FAIL() << "timeout";
}


TEST(ws_server_service_shutdown, plain){
	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ::ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	s.shutdown();

	wait(s);
}

TEST(ws_server_service_shutdown, with_client){
	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ::ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	boost::asio::io_context ioc;
	auto ws = connected_client(ioc);

	s.shutdown();

	read(ws);

	wait(s);
}

TEST(ws_server_service_shutdown, on_open){
	struct ws_service: ::ws_service{
		void on_open(ws_identifier)override{
			executor().shutdown();
		}
	};

	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	boost::asio::io_context ioc;
	auto ws = connected_client(ioc);
	ws.close("");

	wait(s);
}

TEST(ws_server_service_shutdown, on_close){
	struct ws_service: ::ws_service{
		void on_close(ws_identifier)override{
			executor().shutdown();
		}
	};

	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	boost::asio::io_context ioc;
	auto ws = connected_client(ioc);
	ws.close("");

	wait(s);
}

TEST(ws_server_service_shutdown, on_text){
	struct ws_service: ::ws_service{
		void on_text(ws_identifier, std::string&&)override{
			executor().shutdown();
		}

		void on_binary(ws_identifier, std::vector< std::uint8_t >&&)override{
			FAIL() << "binary message";
		}
	};

	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	boost::asio::io_context ioc;
	auto ws = connected_client(ioc);
	ws.text(true);
	ws.write(boost::asio::buffer("a"));
	ws.close("");

	wait(s);
}

TEST(ws_server_service_shutdown, on_binary){
	struct ws_service: ::ws_service{
		void on_text(ws_identifier, std::string&&)override{
			FAIL() << "text message";
		}

		void on_binary(ws_identifier, std::vector< std::uint8_t >&&)override{
			executor().shutdown();
		}
	};

	server s(
		std::make_unique< ::request_handler >(),
		std::make_unique< ws_service >(),
		std::make_unique< ::error_handler >(),
		boost::asio::ip::make_address(host), port, 1);

	boost::asio::io_context ioc;
	auto ws = connected_client(ioc);
	ws.binary(true);
	ws.write(boost::asio::buffer("a"));
	ws.close("");

	wait(s);
}

// TODO: TEST(ws_server_service_shutdown, on_exception);
