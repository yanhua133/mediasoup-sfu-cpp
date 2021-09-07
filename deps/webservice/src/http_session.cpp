//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include "http_session.hpp"
#include "server_impl.hpp"

#include <webservice/server.hpp>
#include <webservice/async_locker.hpp>

#include <boost/beast/websocket.hpp>

#include <boost/asio/bind_executor.hpp>


namespace webservice{


	http_session::http_session(
		boost::asio::ip::tcp::socket&& socket,
		server_impl& server
	)
		: server_(server)
		, socket_(std::move(socket))
		, strand_(boost::asio::make_strand(server_.executor().get_io_context()))
		, timer_(socket_.get_executor(),
			std::chrono::steady_clock::time_point::max())
		, locker_([this]()noexcept{
				server_.http().async_erase(this);
			})
		{}


	// Called when the timer expires.
	void http_session::do_timer(){
		timer_.async_wait(boost::asio::bind_executor(
			strand_,
			[this, lock = locker_.make_lock()](
				boost::system::error_code ec
			){
				if(ec == boost::asio::error::operation_aborted){
					return;
				}

				if(ec){
					server_.http().on_exception(
						std::make_exception_ptr(boost::system::system_error(
							ec, "http session timer")));
				}

				// Closing the socket cancels all outstanding operations.
				// They will complete with operation_aborted
				using socket = boost::asio::ip::tcp::socket;
				socket_.shutdown(socket::shutdown_both, ec);
				socket_.close(ec);
			}));
	}

	void http_session::run(){
		// lock until the first async operations has been started
		auto lock = locker_.make_first_lock();

		// start timer
		do_timer();

		// Start the asynchronous operation
		do_read();
	}

	void http_session::do_read(){
		if(!socket_.is_open()){
			return;
		}

		// Set the timer
		if(timer_.expires_after(server_.http().timeout()) == 0){
			// if the timer could not be cancelled it was already
			// expired and the session was closed by the timer
			return;
		}else{
			// If the timer was cancelled, restart it
			do_timer();
		}

		// Read a request
		boost::beast::http::async_read(socket_, buffer_, req_,
			boost::asio::bind_executor(
				strand_,
				[this, lock = locker_.make_lock()](
					boost::system::error_code ec,
					std::size_t /*bytes_transferred*/
				){
					// Happens when the timer closes the socket
					if(ec == boost::asio::error::operation_aborted){
						return;
					}

					// This means endpoint closed the session
					if(ec == boost::beast::http::error::end_of_stream){
						do_close();
						return;
					}

					if(ec){
						server_.http().on_exception(
							std::make_exception_ptr(boost::system::system_error(
								ec, "http session read")));
						do_close();
						return;
					}

					// See if it is a WebSocket Upgrade
					if(
						server_.has_ws() &&
						boost::beast::websocket::is_upgrade(req_)
					){
						server_.ws().server_connect(
							std::move(socket_), std::move(req_));
						do_close();
					}else{
						// Send the response
						server_.http()(std::move(req_),
							http_response{
								this,
								locker_,
								&http_session::response,
								socket_,
								strand_
							});

						// If we aren't at the queue limit, try to pipeline
						// another request
						if(!queue_.is_full()){
							do_read();
						}else{
							do_close();
						}
					}
				}));
	}

	void http_session::stop_timer()noexcept{
		try{
			timer_.cancel();
		}catch(...){
			server_.http().on_exception(std::current_exception());
		}
	}

	void http_session::on_write(boost::system::error_code ec, bool close){
		// Happens when the timer closes the socket
		if(ec == boost::asio::error::operation_aborted){
			return;
		}

		if(ec){
			server_.http().on_exception(
				std::make_exception_ptr(boost::system::system_error(
					ec, "http session write")));

			do_close();
			return;
		}

		if(close){
			// This means we should close the session, usually because
			// the response indicated the "Connection: close" semantic.
			do_close();
			return;
		}

		// Inform the queue that a write completed
		if(queue_.on_write()){
			// Read another request
			do_read();
		}
	}

	/// \brief Send a TCP shutdown
	void http_session::do_close(){
		boost::system::error_code ec;
		using socket = boost::asio::ip::tcp::socket;
		socket_.shutdown(socket::shutdown_both, ec);
		stop_timer();
	}

	/// \brief Called by the HTTP handler to send a response.
	void http_session::response(
		std::unique_ptr< http_session_work >&& work
	){
		queue_.response(std::move(work));
	}


	http_session::queue::queue():
		items_(limit) {}

	bool http_session::queue::is_full()const{
		return items_.size() >= limit;
	}

	bool http_session::queue::on_write(){
		BOOST_ASSERT(!items_.empty());
		auto const was_full = is_full();
		items_.pop_front();
		if(!items_.empty()){
			(*items_.front())();
		}
		return was_full;
	}

	void http_session::queue::response(
		std::unique_ptr< http_session_work >&& work
	){
		// Allocate and store the work
		items_.push_back(std::move(work));

		// If there was no previous work, start this one
		if(items_.size() == 1){
			(*items_.front())();
		}
	}


	void http_session_on_write::operator()(
		boost::system::error_code ec,
		std::size_t /*bytes_transferred*/
	)const{
		self_->on_write(ec, need_eof_);
	}


}
