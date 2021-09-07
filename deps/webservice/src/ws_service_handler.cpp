//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/ws_service_handler.hpp>
#include <webservice/executor.hpp>

#include <boost/asio/strand.hpp>


namespace webservice{


	/// \brief Implementation of ws_service_handler
	struct ws_service_handler_impl{
		/// \brief Constructor
		ws_service_handler_impl(class executor& executor)
			: strand_(executor.get_executor()) {}


		/// \brief Run async operation sequential
		boost::asio::strand< boost::asio::io_context::executor_type > strand_;

		/// \brief Map type from service name to object
		using service_map =
			std::map< std::string, std::unique_ptr< ws_service_interface > >;

		/// \brief Map from service name to object
		service_map services_;
	};


	ws_service_handler::ws_service_handler() = default;

	ws_service_handler::~ws_service_handler() = default;


	void ws_service_handler::add_service(
		std::string name,
		std::unique_ptr< ws_service_interface > service
	)noexcept try{
		if(!impl_){
			throw std::logic_error(
				"called add_service() before server was set");
		}

		impl_->strand_.dispatch(
			[
				this,
				lock = locker_.make_lock(),
				name = std::move(name),
				service = std::move(service)
			]()mutable noexcept{
				try{
					auto r = impl_->services_.emplace(std::move(name),
						std::move(service));
					if(r.second){
						r.first->second->set_executor(executor());
					}else{
						throw std::logic_error("service(" + name
							+ ") already exists");
					}
				}catch(...){
					on_exception(std::current_exception());
				}
			}, std::allocator< void >());
	}catch(...){
		on_exception(std::current_exception());
	}


	void ws_service_handler::erase_service(std::string name)noexcept try{
		if(!impl_){
			throw std::logic_error(
				"called erase_service() before server was set");
		}

		impl_->strand_.dispatch(
			[
				this,
				lock = locker_.make_lock(),
				name = std::move(name)
			]()mutable noexcept{
				try{
					auto iter = impl_->services_.find(name);
					if(iter != impl_->services_.end()){
						// at this point we don't need lock's because at least
						// the shutdown_lock_ is guaranteed to be active
						iter->second->shutdown(
							[this, iter]()noexcept{
								impl_->strand_.dispatch(
									[this, iter]()noexcept{
										impl_->services_.erase(iter);
										if(
											impl_->services_.empty() &&
											shutdown_
										){
											try{
												shutdown_finished();
											}catch(...){
												on_exception(
													std::current_exception());
											}
										}
									}, std::allocator< void >());
							});
					}else{
						throw std::logic_error("service(" + name
							+ ") doesn't exist");
					}
				}catch(...){
					on_exception(std::current_exception());
				}
			}, std::allocator< void >());
	}catch(...){
		on_exception(std::current_exception());
	}


	void ws_service_handler::on_executor(){
		impl_ = std::make_unique< ws_service_handler_impl >(executor());
	}

	void ws_service_handler::on_server_connect(
		boost::asio::ip::tcp::socket&& socket,
		http_request&& req
	)noexcept try{
		assert(impl_ != nullptr);

		impl_->strand_.dispatch(
			[
				this,
				lock = locker_.make_lock(),
				socket = std::move(socket),
				req = std::move(req)
			]()mutable noexcept{
				try{
					std::string name(req.target());
					auto iter = impl_->services_.find(name);
					if(iter != impl_->services_.end()){
						iter->second->server_connect(std::move(socket),
							std::move(req));
					}else{
						throw std::logic_error("service(" + name
							+ ") doesn't exist");
					}
				}catch(...){
					on_exception(std::current_exception());
				}
			}, std::allocator< void >());
	}catch(...){
		on_exception(std::current_exception());
	}

	void ws_service_handler::on_client_connect(
		std::string&& host,
		std::string&& port,
		std::string&& resource
	)noexcept try{
		assert(impl_ != nullptr);

		impl_->strand_.dispatch(
			[
				this,
				lock = locker_.make_lock(),
				host = std::move(host),
				port = std::move(port),
				resource = std::move(resource)
			]()mutable noexcept{
				try{
					auto iter = impl_->services_.find(resource);
					if(iter != impl_->services_.end()){
						iter->second->client_connect(std::move(host),
							std::move(port), std::move(resource));
					}else{
						throw std::logic_error("service(" + resource
							+ ") doesn't exist");
					}
				}catch(...){
					on_exception(std::current_exception());
				}
			}, std::allocator< void >());
	}catch(...){
		on_exception(std::current_exception());
	}

	void ws_service_handler::on_shutdown()noexcept try{
		assert(impl_ != nullptr);

		impl_->strand_.defer(
			[
				this,
				lock = locker_.make_lock()
			]()mutable noexcept{
				if(impl_->services_.empty()){
					try{
						shutdown_finished();
					}catch(...){
						on_exception(std::current_exception());
					}
				}else{
					for(auto& service: impl_->services_){
						service.second->shutdown();
					}
				}

				shutdown_ = true;
			}, std::allocator< void >());
	}catch(...){
		on_exception(std::current_exception());
	}



}
