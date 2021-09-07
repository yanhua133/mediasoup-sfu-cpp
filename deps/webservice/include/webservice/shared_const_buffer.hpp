//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__shared_const_buffer__hpp_INCLUDED_
#define _webservice__shared_const_buffer__hpp_INCLUDED_

#include <boost/asio/buffer.hpp>

#include <boost/any.hpp>

#include <boost/optional.hpp>

#include <memory>


namespace webservice{


	struct construct_via_data_and_size_t{};

	constexpr construct_via_data_and_size_t construct_via_data_and_size{};

	/// \brief A generic reference-counted non-modifiable buffer class
	class shared_const_buffer{
	public:
		/// \brief Move or copy the data to a shared_ptr
		template < typename T >
		explicit shared_const_buffer(T&& data)
			: shared_const_buffer(
				std::make_shared< typename std::decay< T >::type const >(
					static_cast< T&& >(data))) {}

		/// \brief Use the shared_ptr directly
		template < typename T >
		explicit shared_const_buffer(std::shared_ptr< T > data)
			: data_(std::move(data))
			, buffer_(boost::asio::buffer(
				*boost::any_cast< std::shared_ptr< T > >(data_))) {}

		/// \brief Move or copy the data to a shared_ptr
		template < typename T >
		explicit shared_const_buffer(T&& data, construct_via_data_and_size_t c)
			: shared_const_buffer(
				std::make_shared< typename std::decay< T >::type const >(
					static_cast< T&& >(data)), c) {}

		/// \brief Use the shared_ptr directly
		template < typename T >
		explicit shared_const_buffer(
			std::shared_ptr< T > data,
			construct_via_data_and_size_t
		)
			: data_(std::move(data))
			, buffer_(boost::asio::buffer(
					static_cast< void const* >(
						boost::any_cast< std::shared_ptr< T > >(data_)->data()
					),
					boost::any_cast< std::shared_ptr< T > >(data_)->size() *
					sizeof(*data->data())
				)) {}


		/// \brief Buffer interface value_type
		using value_type = boost::asio::const_buffer;

		/// \brief Buffer interface const_iterator
		using const_iterator = boost::asio::const_buffer const*;


		/// \brief Buffer interface begin
		boost::asio::const_buffer const* begin()const{
			return &buffer_;
		}

		/// \brief Buffer interface end
		boost::asio::const_buffer const* end()const{
			return &buffer_ + 1;
		}


	private:
		/// \brief A shared_ptr that keeps the buffer data alive
		boost::any data_;

		/// \brief Buffer representation of the data
		boost::asio::const_buffer buffer_;
	};


}


#endif
