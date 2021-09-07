//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "server_impl.hpp"
#include "http_session.hpp"
#include "http_sessions.hpp"

#include <webservice/http_request_handler.hpp>

#include <boost/beast/version.hpp>


namespace webservice{


	namespace http = boost::beast::http;


	http_request_handler::http_request_handler() = default;

	http_request_handler::~http_request_handler() = default;


	void http_request_handler::async_emplace(
		boost::asio::ip::tcp::socket&& socket
	)noexcept{
		assert(list_ != nullptr);

		list_->async_emplace(std::move(socket));
	}

	void http_request_handler::async_erase(http_session* session)noexcept{
		assert(list_ != nullptr);

		list_->async_erase(session);
	}

	void http_request_handler::operator()(
		http_request&& req,
		http_response&& send
	){
		send(not_found(req, req.target()));
	}

	void http_request_handler::on_exception(
		std::exception_ptr /*error*/)noexcept{}

	void http_request_handler::on_shutdown()noexcept{}

	void http_request_handler::shutdown()noexcept{
		assert(list_ != nullptr);

		list_->shutdown();
		on_shutdown();
	}

	bool http_request_handler::is_shutdown()noexcept{
		return list_ && list_->is_shutdown();
	}


	http_string_response bad_request(
		http_request const& req,
		boost::beast::string_view why
	){
		http_string_response res{http::status::bad_request, req.version()};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = why.to_string();
		res.prepare_payload();
		return res;
	}

	http_string_response not_found(
		http_request const& req,
		boost::beast::string_view target
	){
		http_string_response res{http::status::not_found, req.version()};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + target.to_string() + "' was not found.";
		res.prepare_payload();
		return res;
	}

	http_string_response server_error(
		http_request const& req,
		boost::beast::string_view what
	){
		http_string_response res{
			http::status::internal_server_error, req.version()};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + what.to_string() + "'";
		res.prepare_payload();
		return res;
	}


	void http_request_handler::set_server(server_impl& server){
		list_ = std::make_unique< http_sessions >(server);
	}

	class server& http_request_handler::server(){
		if(!list_){
			throw std::logic_error("called sever() before set_server");
		}

		return list_->server();
	}


}
