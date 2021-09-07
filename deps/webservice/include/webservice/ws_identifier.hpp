//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__ws_identifier__hpp_INCLUDED_
#define _webservice__ws_identifier__hpp_INCLUDED_

#include <ostream>


namespace webservice{


	class ws_session;
	class ws_handler_interface;
	class shared_const_buffer;


	/// \brief Wrapper around a websocket server session
	///
	/// A ws_identifier is always bound to a ws_session. If you need an empty
	/// state use optional_ws_identifier.
	class ws_identifier{
	private:
		/// \brief Constructor
		explicit constexpr ws_identifier(ws_session& session)noexcept
			: session(&session) {}


		/// \brief The corresponding session
		ws_session* session;


		template < typename CharT, typename Traits >
		friend std::basic_ostream< CharT, Traits >& operator<<(
			std::basic_ostream< CharT, Traits >& os,
			ws_identifier identifier
		){
			return os << identifier.session;
		}


		friend constexpr bool operator==(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session == r.session;
		}

		friend constexpr bool operator!=(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session != r.session;
		}

		friend constexpr bool operator<(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session < r.session;
		}

		friend constexpr bool operator>(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session > r.session;
		}

		friend constexpr bool operator<=(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session <= r.session;
		}

		friend constexpr bool operator>=(
			ws_identifier l,
			ws_identifier r
		)noexcept{
			return l.session >= r.session;
		}


		template < typename Value >
		friend class ws_service_base;
		friend class ws_session;
		friend class ws_sessions;
		friend class optional_ws_identifier;
	};


	/// \brief Same as ws_identifier but with an empty state
	class optional_ws_identifier{
	public:
		/// \brief Construct with empty state
		constexpr optional_ws_identifier()noexcept
			: session(nullptr) {}

		/// \brief Construct with same session as identifier
		constexpr optional_ws_identifier(ws_identifier identifier)noexcept
			: session(identifier.session) {}


		/// \brief Converts to ws_identifier
		///
		/// \throw std::runtime_error if it was in empty state
		explicit constexpr operator ws_identifier(){
			if(session != nullptr){
				return ws_identifier(*session);
			}else{
				throw std::runtime_error(
					"empty optional_ws_identifier converted to ws_identifier");
			}
		}


		/// \brief true if state is not empty
		constexpr operator bool()noexcept{
			return session != nullptr;
		}


	private:
		/// \brief The corresponding session
		ws_session* session;


		template < typename CharT, typename Traits >
		friend std::basic_ostream< CharT, Traits >& operator<<(
			std::basic_ostream< CharT, Traits >& os,
			optional_ws_identifier identifier
		){
			return os << identifier.session;
		}


		friend constexpr bool operator==(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session == r.session;
		}

		friend constexpr bool operator!=(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session != r.session;
		}

		friend constexpr bool operator<(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session < r.session;
		}

		friend constexpr bool operator>(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session > r.session;
		}

		friend constexpr bool operator<=(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session <= r.session;
		}

		friend constexpr bool operator>=(
			optional_ws_identifier l,
			optional_ws_identifier r
		)noexcept{
			return l.session >= r.session;
		}
	};



}


#endif
