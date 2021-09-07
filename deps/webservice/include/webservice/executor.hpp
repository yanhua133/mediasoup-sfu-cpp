//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__executor__hpp_INCLUDED_
#define _webservice__executor__hpp_INCLUDED_

#include "error_handler.hpp"

#include <boost/asio/io_context.hpp>

#include <functional>
#include <vector>
#include <thread>
#include <mutex>


namespace webservice{


	/// \brief Base class for server error handlers
	class executor{
	public:
		/// \brief Constructor
		template < typename ShutdownFn >
		executor(
			boost::asio::io_context& ioc,
			std::unique_ptr< error_handler >&& handler,
			ShutdownFn&& shutdown_fn
		)
			: ioc_(ioc)
			, error_handler_(std::move(handler))
			, shutdown_fn_(static_cast< ShutdownFn&& >(shutdown_fn))
		{
			static_assert(noexcept(static_cast< ShutdownFn&& >(shutdown_fn)),
				"shutdown_fn must be noexcept");

			// Create the default error_handler if none exist
			if(!error_handler_){
				error_handler_ = std::make_unique< class error_handler >();
			}
		}


		executor(executor const&) = delete;

		executor& operator=(executor const&) = delete;


		/// \brief Destructor
		~executor();


		/// \brief Run the io_context on all threads
		void run(std::uint8_t thread_count);


		/// \brief Wait on all processing threads
		///
		/// This effecivly blocks the current thread until the server_impl is
		/// closed.
		void block()noexcept;

		/// \brief Don't accept new connections and async tasks
		///
		/// This function is not blocking. Call block() if you want to wait
		/// until all connections are closed.
		void shutdown()noexcept;


		/// \brief Get executor
		boost::asio::io_context::executor_type get_executor();

		/// \brief Get reference to the internal io_context
		boost::asio::io_context& get_io_context()noexcept;

		/// \brief true after all tasks have finished
		bool is_stopped()noexcept;


		/// \brief Reference to the error_handler
		error_handler& error()const{
			return *error_handler_;
		}


	private:
		/// \brief Reference to the io_context
		boost::asio::io_context& ioc_;

		/// \brief Handles errors and exceptions in the server
		std::unique_ptr< error_handler > error_handler_;

		/// \brief Called on the first call of shutdown()
		std::function< void() > shutdown_fn_;

		/// \brief Makes sure shutdown_fn_ is called only one time
		std::once_flag shutdown_flag_;

		/// \brief Protect thread joins
		std::mutex mutex_;

		/// \brief The worker threads
		std::vector< std::thread > threads_;
	};


}


#endif
