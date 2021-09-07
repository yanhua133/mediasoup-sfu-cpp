//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__basic_ws_service__hpp_INCLUDED_
#define _webservice__basic_ws_service__hpp_INCLUDED_

#include "ws_service_base.hpp"
#include "conversion.hpp"


namespace webservice{


	/// \brief Dummy data that can be used as value if you don't need a value
	struct none_t{};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
	template <
		typename Value,
		typename SendTextType,
		typename SendBinaryType = SendTextType,
		typename ReceiveTextType = SendTextType,
		typename ReceiveBinaryType = SendBinaryType >
	class basic_ws_service: public ws_service_base< Value >{
	public:
		static constexpr to_shared_const_buffer_t< SendTextType >
			text_to_shared_const_buffer{};

		static constexpr to_shared_const_buffer_t< SendBinaryType >
			binary_to_shared_const_buffer{};

		static constexpr from_multi_buffer_t< ReceiveTextType >
			multi_buffer_to_text{};

		static constexpr from_multi_buffer_t< ReceiveBinaryType >
			multi_buffer_to_binary{};


		using ws_service_base< Value >::ws_service_base;


		/// \brief Send a text message to all sessions
		template < typename SendTextTypeT >
		void send_text(SendTextTypeT&& data){
			ws_service_base< Value >::send_text(
				text_to_shared_const_buffer(
					static_cast< SendTextTypeT&& >(data)));
		}

		/// \brief Send a text message to session
		template < typename SendTextTypeT >
		void send_text(ws_identifier identifier, SendTextTypeT&& data){
			ws_service_base< Value >::send_text(
				identifier, text_to_shared_const_buffer(
					static_cast< SendTextTypeT&& >(data)));
		}

		/// \brief Send a text message to session
		template < typename UnaryFunction, typename SendTextTypeT >
		void send_text_if(UnaryFunction fn, SendTextTypeT&& data){
			ws_service_base< Value >::send_text_if(
				std::move(fn), text_to_shared_const_buffer(
					static_cast< SendTextTypeT&& >(data)));
		}


		/// \brief Send a binary message to all sessions
		template < typename SendBinaryTypeT >
		void send_binary(SendBinaryTypeT&& data){
			ws_service_base< Value >::send_binary(
				binary_to_shared_const_buffer(
					static_cast< SendBinaryTypeT&& >(data)));
		}

		/// \brief Send a binary message to session
		template < typename SendBinaryTypeT >
		void send_binary(ws_identifier identifier, SendBinaryTypeT&& data){
			ws_service_base< Value >::send_binary(
				identifier, binary_to_shared_const_buffer(
					static_cast< SendBinaryTypeT&& >(data)));
		}

		/// \brief Send a binary message to session
		template < typename UnaryFunction, typename SendBinaryTypeT >
		void send_binary_if(UnaryFunction fn, SendBinaryTypeT&& data){
			ws_service_base< Value >::send_binary_if(
				std::move(fn), binary_to_shared_const_buffer(
					static_cast< SendBinaryTypeT&& >(data)));
		}


	private:
		/// \brief Called when a session received a text message
		///
		/// Default implementation does nothing.
		virtual void on_text(
			ws_identifier /*identifier*/,
			ReceiveTextType&& /*data*/){}

		/// \brief Called when a session received a binary message
		///
		/// Default implementation does nothing.
		virtual void on_binary(
			ws_identifier /*identifier*/,
			ReceiveBinaryType&& /*data*/){}



		/// \brief Called when a session received a text message
		///
		/// Default implementation does nothing.
		void on_text(
			ws_identifier identifier,
			boost::beast::multi_buffer&& buffer
		)final{
			try{
				on_text(identifier, multi_buffer_to_text(buffer));
			}catch(...){
				this->on_exception(identifier, std::current_exception());
			}
		}

		/// \brief Called when a session received a binary message
		///
		/// Default implementation does nothing.
		void on_binary(
			ws_identifier identifier,
			boost::beast::multi_buffer&& buffer
		)final{
			try{
				on_binary(identifier, multi_buffer_to_binary(buffer));
			}catch(...){
				this->on_exception(identifier, std::current_exception());
			}
		}
	};
#ifdef __clang__
#pragma clang diagnostic pop
#endif


	template <
		typename Value,
		typename SendTextType,
		typename SendBinaryType,
		typename ReceiveTextType,
		typename ReceiveBinaryType >
	constexpr to_shared_const_buffer_t< SendTextType > basic_ws_service<
		Value,
		SendTextType,
		SendBinaryType,
		ReceiveTextType,
		ReceiveBinaryType >::text_to_shared_const_buffer;

	template <
		typename Value,
		typename SendTextType,
		typename SendBinaryType,
		typename ReceiveTextType,
		typename ReceiveBinaryType >
	constexpr to_shared_const_buffer_t< SendBinaryType > basic_ws_service<
		Value,
		SendTextType,
		SendBinaryType,
		ReceiveTextType,
		ReceiveBinaryType >::binary_to_shared_const_buffer;

	template <
		typename Value,
		typename SendTextType,
		typename SendBinaryType,
		typename ReceiveTextType,
		typename ReceiveBinaryType >
	constexpr from_multi_buffer_t< ReceiveTextType > basic_ws_service<
		Value,
		SendTextType,
		SendBinaryType,
		ReceiveTextType,
		ReceiveBinaryType >::multi_buffer_to_text;

	template <
		typename Value,
		typename SendTextType,
		typename SendBinaryType,
		typename ReceiveTextType,
		typename ReceiveBinaryType >
	constexpr from_multi_buffer_t< ReceiveBinaryType > basic_ws_service<
		Value,
		SendTextType,
		SendBinaryType,
		ReceiveTextType,
		ReceiveBinaryType >::multi_buffer_to_binary;


}


#endif
