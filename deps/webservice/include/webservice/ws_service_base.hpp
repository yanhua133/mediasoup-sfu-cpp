//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_service_base__hpp_INCLUDED_
#define _webservice__ws_service_base__hpp_INCLUDED_

#include "shared_const_buffer.hpp"
#include "ws_service_interface.hpp"
#include "ws_session_settings.hpp"
#include "ws_session.hpp"
#include "executor.hpp"
#include "server.hpp"

#include <string>
#include <set>

#include <boost/asio/connect.hpp>


namespace webservice{


	/// \brief Base for any server websocket service that handels standing
	///        sessions
	///
	/// If you derive from this class you have to override on_server_connect
	/// calling async_server_connect and / or on_client_connect calling
	/// async_client_connect with additional ValueArgs that are used as
	/// arguments for the constructor of Value.
	template < typename Value >
	class ws_service_base
		: public ws_service_interface
		, public ws_session_settings{
	public:
		static_assert(std::is_move_constructible< Value >::value,
			"Value must be move constructible");


		/// \brief Constructor
		ws_service_base() = default;


		ws_service_base(ws_service_base const&) = delete;

		ws_service_base& operator=(ws_service_base const&) = delete;


		/// \brief Send a text message to session
		void send_text(
			ws_identifier identifier,
			shared_const_buffer buffer
		){
			if(!impl_){
				throw std::logic_error(
					"called send_text() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					identifier,
					buffer = std::move(buffer)
				]()mutable noexcept{
					if(impl_->map_.count(identifier) > 0){
						identifier.session->send(true, std::move(buffer));
					}
				}, std::allocator< void >());
		}

		/// \brief Send a text message to all session
		void send_text(shared_const_buffer buffer){
			send_text_if([](ws_identifier, Value&)noexcept{
					return true;
				}, buffer);
		}

		/// \brief Send a text message to all sessions for which fn(value)
		///        returns true
		///
		/// The value in fn(value) is the data linked to the session.
		template < typename UnaryFunction >
		void send_text_if(
			UnaryFunction fn,
			shared_const_buffer buffer
		){
			if(!impl_){
				throw std::logic_error(
					"called send_text_if() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					fn = std::move(fn),
					buffer = std::move(buffer)
				]()mutable noexcept{
					for(auto& session: impl_->map_){
						ws_identifier identifier(strip_const(session.first));
						try{
							if(fn(identifier, session.second)){
								identifier.session->send(true, buffer);
							}
						}catch(...){
							on_exception(identifier, std::current_exception());
						}
					}
				}, std::allocator< void >());
		}


		/// \brief Send a binary message to session
		void send_binary(
			ws_identifier identifier,
			shared_const_buffer buffer
		){
			if(!impl_){
				throw std::logic_error(
					"called send_binary() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					identifier,
					buffer = std::move(buffer)
				]()mutable noexcept{
					if(impl_->map_.count(identifier) > 0){
						identifier.session->send(false, std::move(buffer));
					}
				}, std::allocator< void >());
		}

		/// \brief Send a binary message to all session
		void send_binary(shared_const_buffer buffer){
			send_binary_if([](ws_identifier, Value&)noexcept{
					return true;
				}, buffer);
		}

		/// \brief Send a binary message to all sessions for which fn(value)
		///        returns true
		///
		/// The value in fn(value) is the data linked to the session.
		template < typename UnaryFunction >
		void send_binary_if(
			UnaryFunction fn,
			shared_const_buffer buffer
		){
			if(!impl_){
				throw std::logic_error(
					"called send_binary_if() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					fn = std::move(fn),
					buffer = std::move(buffer)
				]()mutable noexcept{
					for(auto& session: impl_->map_){
						ws_identifier identifier(strip_const(session.first));
						try{
							if(fn(identifier, session.second)){
								identifier.session->send(false, buffer);
							}
						}catch(...){
							on_exception(identifier, std::current_exception());
						}
					}
				}, std::allocator< void >());
		}


		/// \brief Shutdown session
		void close(
			ws_identifier identifier,
			boost::beast::websocket::close_reason reason
		){
			if(!impl_){
				throw std::logic_error("called close() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					identifier,
					reason = std::move(reason)
				]()mutable noexcept{
					if(impl_->map_.count(identifier) > 0){
						identifier.session->close(reason);
					}

				}, std::allocator< void >());
		}

		/// \brief Shutdown all sessions
		void close(boost::beast::websocket::close_reason reason){
			close_if([](ws_identifier, Value&)noexcept{
					return true;
				}, reason);
		}

		/// \brief Send a close to all sessions for which fn(value)
		///        returns true
		///
		/// The value in fn(value) is the data linked to the session.
		template < typename UnaryFunction >
		void close_if(
			UnaryFunction fn,
			boost::beast::websocket::close_reason reason
		){
			if(!impl_){
				throw std::logic_error(
					"called close_if() before server was set");
			}

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					fn = std::move(fn),
					reason = std::move(reason)
				]()mutable noexcept{
					for(auto& session: impl_->map_){
						ws_identifier identifier(strip_const(session.first));
						try{
							if(fn(identifier, session.second)){
								identifier.session->close(reason);
							}
						}catch(...){
							on_exception(identifier, std::current_exception());
						}
					}
				}, std::allocator< void >());
		}


		/// \brief Modify value of identifier via the given function async
		template < typename Fn >
		void modify_value(ws_identifier identifier, Fn fn){
			assert(impl_ != nullptr);

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					identifier,
					fn = std::move(fn)
				]()mutable noexcept{
					auto iter = impl_->map_.find(identifier);
					if(iter != impl_->map_.end()){
						try{
							fn(iter->second);
						}catch(...){
							on_exception(identifier, std::current_exception());
						}
					}
				}, std::allocator< void >());
		}

		/// \brief Set value of identifier to given value async
		void set_value(ws_identifier identifier, Value value){
			modify_value(
				identifier,
				[value = std::move(value)](Value& value_ref)mutable{
					value_ref = std::move(value);
				});
		}


	protected:
		/// \brief Create the implementation
		///
		/// \attention: If you override on_executor(), call this from your
		///             overriding function.
		void on_executor()override{
			impl_ = std::make_unique< impl >(this->executor().get_executor());
		}

		/// \brief Accept no new sessions, send close to all session
		///
		/// \attention: If you override on_shutdown(), call this from your
		///             overriding function. Is this case you may have to
		///             override on_shutdown_finished() also.
		void on_shutdown()noexcept override{
			assert(impl_ != nullptr);

			impl_->strand_.defer(
				[this, lock = locker_.make_lock()]{
					if(impl_->map_.empty()){
						try{
							on_shutdown_finished();
						}catch(...){
							on_exception(std::current_exception());
						}
					}else{
						for(auto& session: impl_->map_){
							strip_const(session.first).close("shutdown");
						}
					}

					shutdown_ = true;
				}, std::allocator< void >());
		}

		/// \brief Erase the session from map_ async
		///
		/// \attention: If you override on_shutdown(), call this from your
		///             overriding function. Is this case you may have to
		///             override on_shutdown_finished() also.
		void on_erase(ws_identifier identifier)noexcept override{
			assert(impl_ != nullptr);

			impl_->strand_.dispatch(
				[this, lock = locker_.make_lock(), identifier]()noexcept{
					try{
						auto iter = impl_->map_.find(identifier);
						if(iter == impl_->map_.end()){
							throw std::logic_error("session doesn't exist");
						}

						try{
							on_value_erase(identifier, std::move(iter->second));
						}catch(...){
							on_exception(identifier, std::current_exception());
						}

						impl_->map_.erase(iter);

						// note shutdown_ is used, not is_shutdown()
						if(impl_->map_.empty() && shutdown_){
							on_shutdown_finished();
						}
					}catch(...){
						on_exception(identifier, std::current_exception());
					}
				}, std::allocator< void >());
		}


		/// \brief Called directly before erasure with the last value state
		///
		/// Default Implementation does nothing.
		virtual void on_value_erase(ws_identifier, Value&&){}


		/// \brief Create a new session async
		///
		/// Call this function from your on_server_connect() overriding
		/// function to create a new websocket server session.
		template < typename ... ValueArgs >
		void async_server_connect(
			boost::asio::ip::tcp::socket&& socket,
			http_request&& req,
			ValueArgs&& ... args
		){
			assert(impl_ != nullptr);

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					socket = std::move(socket),
					req = std::move(req),
					args = std::make_tuple(static_cast< ValueArgs&& >(args) ...)
				]()mutable noexcept{
					try{
						if(is_shutdown()){
							throw std::logic_error(
								"emplace in ws_service_base while shutdown");
						}

						ws_stream ws(std::move(socket));
						ws.read_message_max(max_read_message_size());

						auto iter = impl_->map_.emplace(
							std::piecewise_construct,
							std::forward_as_tuple(std::move(ws), *this,
								ping_time()), std::move(args));

						ws_identifier identifier(
							strip_const(iter.first->first));
						try{
							identifier.session->do_accept(std::move(req));
						}catch(...){
							on_exception(identifier, std::current_exception());
							on_erase(identifier);
						}
					}catch(...){
						on_exception(std::current_exception());
					}
				}, std::allocator< void >());
		}


		/// \brief Create a new session async
		///
		/// Call this function from your on_client_connect() overriding
		/// function to create a new websocket client session.
		template < typename ... ValueArgs >
		void async_client_connect(
			std::string host,
			std::string port,
			std::string resource,
			ValueArgs&& ... args
		){
			assert(impl_ != nullptr);

			impl_->strand_.dispatch(
				[
					this,
					lock = locker_.make_lock(),
					host = std::move(host),
					port = std::move(port),
					resource = std::move(resource),
					args = std::make_tuple(static_cast< ValueArgs&& >(args) ...)
				]()mutable noexcept{
					try{
						if(is_shutdown()){
							throw std::logic_error(
								"emplace in ws_service_base while shutdown");
						}

						boost::asio::ip::tcp::resolver resolver(
							executor().get_io_context());
						auto results = resolver.resolve(host, port);

						ws_stream ws(executor().get_io_context());
						ws.read_message_max(max_read_message_size());

						// Make the session on the IP address we get from a
						// lookup
						boost::asio::connect(ws.next_layer(),
							results.begin(), results.end());

						// Perform the ws handshake
						ws.handshake(host, resource);

						auto iter = impl_->map_.emplace(
							std::piecewise_construct,
							std::forward_as_tuple(std::move(ws), *this,
								ping_time()), std::move(args));

						ws_identifier identifier(
							strip_const(iter.first->first));
						try{
							identifier.session->start();
						}catch(...){
							on_exception(identifier, std::current_exception());
							on_erase(identifier);
						}
					}catch(...){
						on_exception(std::current_exception());
					}
				}, std::allocator< void >());
		}


	private:
		struct less{
			using is_transparent = void;

			bool operator()(
				ws_session const& l,
				ws_session const& r
			)const noexcept{
				return &l < &r;
			}

			bool operator()(
				ws_session* l,
				ws_session const& r
			)const noexcept{
				return l < &r;
			}

			bool operator()(
				ws_session const& l,
				ws_session* r
			)const noexcept{
				return &l < r;
			}

			bool operator()(
				ws_identifier l,
				ws_session const& r
			)const noexcept{
				return l.session < &r;
			}

			bool operator()(
				ws_session const& l,
				ws_identifier r
			)const noexcept{
				return &l < r.session;
			}
		};

		/// \brief Called when all sessions have been erased after shutdown
		///
		/// Default implementation calls shutdown_finished(). Override it, if
		/// you need to call shutdown_finished() yourself.
		virtual void on_shutdown_finished(){
			shutdown_finished();
		}

		/// \brief Remove const from session
		///
		/// Map keys are always const, but we need non const session objects.
		/// The const can be safely removed because map order is done by
		/// address and the address can not change even for a non-const object.
		static constexpr ws_session& strip_const(
			ws_session const& session
		)noexcept{
			return const_cast< ws_session& >(session);
		}

		/// \brief Implementation data after the executor was set
		struct impl{
			impl(boost::asio::io_context::executor_type&& executor)
				: strand_(std::move(executor)) {}

			strand strand_;
			std::map< ws_session, Value, less > map_;
		};


		/// \brief true after on_shutdown async has finished
		///
		/// \attention This is not equivalent with is_shutdown().
		bool shutdown_{false};

		/// \brief Pointer to implementation
		std::unique_ptr< impl > impl_;
	};


}


#endif
