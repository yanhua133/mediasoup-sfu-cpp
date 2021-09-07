//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "http_sessions.hpp"
#include "http_session.hpp"
#include "server_impl.hpp"


namespace webservice{


	http_sessions::http_sessions(server_impl& server)
		: server_(server)
		, run_lock_(locker_.make_first_lock())
		, strand_(server_.executor().get_executor()) {}


	class server& http_sessions::server()const noexcept{
		return server_.server();
	}


	void http_sessions::async_emplace(
		boost::asio::ip::tcp::socket&& socket
	)noexcept try{
		strand_.dispatch(
			[
				this,
				lock = locker_.make_lock(),
				socket = std::move(socket)
			]()mutable{
				if(is_shutdown()){
					throw std::logic_error(
						"emplace in http_sessions while shutdown");
				}

				auto session = std::make_unique< http_session >(
					std::move(socket), server_);

				auto iter = set_.insert(set_.end(), std::move(session));

				try{
					(*iter)->run();
				}catch(...){
					async_erase(session.get());
					throw;
				}
			}, std::allocator< void >());
	}catch(...){
		server_.http().on_exception(std::current_exception());
	}

	void http_sessions::async_erase(http_session* session)noexcept try{
		strand_.dispatch(
			[this, lock = locker_.make_lock(), session]{
				auto iter = set_.find(session);
				if(iter == set_.end()){
					throw std::logic_error("session doesn't exist");
				}
				set_.erase(iter);

				if(set_.empty() && is_shutdown()){
					shutdown_lock_.unlock();
				}
			}, std::allocator< void >());
	}catch(...){
		server_.http().on_exception(std::current_exception());
	}


	void http_sessions::shutdown()noexcept{
		auto lock = std::move(run_lock_);
		if(lock.is_locked()){
			shutdown_lock_ = std::move(lock);

			strand_.defer(
				[
					this,
					lock = locker_.make_lock()
				]()mutable{
					if(set_.empty()){
						shutdown_lock_.unlock();
					}else{
						for(auto& session: set_){
							session->do_close();
						}
					}
				}, std::allocator< void >());
		}
	}

	bool http_sessions::is_shutdown()noexcept{
		return !run_lock_.is_locked();
	}


}
