//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/executor.hpp>


namespace webservice{


	executor::~executor(){
		assert(threads_.empty());
	}

	void executor::run(std::uint8_t thread_count){
		// Run the I/O service on the requested number of thread_count
		threads_.reserve(thread_count);
		for(std::size_t i = 0; i < thread_count; ++i){
			threads_.emplace_back([this]{
				// restart io_context if it returned by exception
				for(;;){
					try{
						ioc_.run();
						return;
					}catch(...){
						error_handler_->on_exception(std::current_exception());
					}
				}
			});
		}
	}


	void executor::block()noexcept{
		std::lock_guard< std::mutex > lock(mutex_);
		for(auto& thread: threads_){
			if(thread.joinable()){
				try{
					thread.join();
				}catch(...){
					error_handler_->on_exception(std::current_exception());
				}
			}
		}

		threads_.clear();
	}

	bool executor::is_stopped()noexcept{
		return ioc_.stopped();
	}

	void executor::shutdown()noexcept{
		std::call_once(shutdown_flag_, shutdown_fn_);
	}


	boost::asio::io_context::executor_type executor::get_executor(){
		return ioc_.get_executor();
	}

	boost::asio::io_context& executor::get_io_context()noexcept{
		return ioc_;
	}


}
