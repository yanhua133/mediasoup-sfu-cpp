//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__listener__hpp_INCLUDED_
#define _webservice__listener__hpp_INCLUDED_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace webservice{


	/// \brief Accepts incoming connections and launches the sessions
	class listener{
	public:
		/// \brief Constructor
		listener(
			class server_impl& server,
			boost::asio::ip::tcp::endpoint endpoint,
			boost::asio::io_context& ioc
		);


		/// \brief Start to accept sessions async
		///
		/// This function is not blocking.
		void do_accept();

		/// \brief Stop to accept session
		void shutdown()noexcept;


	private:
		/// \brief Reference to the server
		class server_impl& server_;

		/// \brief The acceptor
		boost::asio::ip::tcp::acceptor acceptor_;

		/// \brief The socket
		boost::asio::ip::tcp::socket socket_;
	};


}


#endif
