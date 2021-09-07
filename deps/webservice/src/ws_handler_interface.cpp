//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/ws_handler_interface.hpp>


namespace webservice{


	ws_handler_interface::ws_handler_interface()noexcept
		: run_lock_(locker_.make_first_lock()) {}


	void ws_handler_interface::set_executor(class executor& executor)noexcept{
		executor_ = &executor;
		on_executor();
	}

	void ws_handler_interface::server_connect(
		boost::asio::ip::tcp::socket&& socket,
		http_request&& req
	){
		on_server_connect(std::move(socket), std::move(req));
	}

	void ws_handler_interface::client_connect(
		std::string host,
		std::string port,
		std::string resource
	){
		on_client_connect(std::move(host), std::move(port),
			std::move(resource));
	}

	void ws_handler_interface::shutdown()noexcept{
		if(!shutdown_lock_.is_locked()){
			shutdown_lock_ = std::move(run_lock_);
			if(shutdown_lock_.is_locked()){
				on_shutdown();
			}
		}
	}

	void ws_handler_interface::shutdown_finished(){
		if(run_lock_.is_locked()){
			throw std::logic_error("shutdown_finished() call without shutdown");
		}

		auto lock = std::move(shutdown_lock_);
		if(!lock.is_locked()){
			throw std::logic_error("repeated shutdown_finished() call");
		}
	}


	class executor& ws_handler_interface::executor(){
		if(!executor_){
			throw std::logic_error("called executor() before set_executor()");
		}

		return *executor_;
	}


	void ws_handler_interface::on_exception(
		std::exception_ptr /*error*/)noexcept{}


	void ws_handler_interface::on_server_connect(
		boost::asio::ip::tcp::socket&& /*socket*/,
		http_request&& /*req*/
	){
		throw std::logic_error(
			"on_server_connect() is not overriden by your websocket service");
	}

	void ws_handler_interface::on_client_connect(
		std::string&& /*host*/,
		std::string&& /*port*/,
		std::string&& /*resource*/
	){
		throw std::logic_error(
			"on_client_connect() is not overriden by your websocket service");
	}


	bool ws_handler_interface::is_shutdown()noexcept{
		return !run_lock_.is_locked();
	}


}
