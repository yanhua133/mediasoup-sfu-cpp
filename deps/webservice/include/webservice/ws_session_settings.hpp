//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_session_settings__hpp_INCLUDED_
#define _webservice__ws_session_settings__hpp_INCLUDED_

#include <chrono>


namespace webservice{


	/// \brief Settings for websocket sessions
	class ws_session_settings{
	public:
		/// \brief Set max size of incomming WebSocket messages
		void set_max_read_message_size(std::size_t bytes){
			max_read_message_size_ = bytes;
		}

		/// \brief Max size of incomming WebSocket messages
		std::size_t max_read_message_size()const{
			return max_read_message_size_;
		}


		/// \brief Set session timeout
		void set_ping_time(std::chrono::milliseconds ms){
			ping_time_ = ms;
		}

		/// \brief Session timeout
		std::chrono::milliseconds ping_time()const{
			return ping_time_;
		}


	private:
		/// \brief Max size of incomming http and WebSocket messages
		std::size_t max_read_message_size_{16 * 1024 * 1024};

		/// \brief WebSocket session timeout
		///
		/// After this time without an incomming message a ping is send.
		/// If no message is incomming after a second period of this time, the
		/// session is considerd to be dead and will be closed.
		std::chrono::milliseconds ping_time_{15000};
	};


}


#endif
