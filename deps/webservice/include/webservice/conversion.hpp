//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__conversion__hpp_INCLUDED_
#define _webservice__conversion__hpp_INCLUDED_

#include "shared_const_buffer.hpp"

#include <boost/beast/core/multi_buffer.hpp>

#include <type_traits>
#include <utility>


namespace webservice{


	namespace detail{


		template < typename T, typename = void >
		struct buffers_constructible: std::false_type{};

		template < typename T >
		struct buffers_constructible< T,
				decltype((void)boost::asio::buffer(std::declval< T >())) >
			: std::true_type{};


		template < typename T, typename = void >
		struct has_insert_function: std::false_type{};

		template < typename T >
		struct has_insert_function< T,
				decltype((void)std::declval< T >().insert(
					std::declval< T >().end(),
					std::declval< typename T::value_type const* >(),
					std::declval< typename T::value_type const* >()
				)) >
			: std::true_type{};


		template < typename T, typename = void >
		struct call_reserve{
			void operator()(T&, std::size_t)const{}
		};

		template < typename T >
		struct call_reserve< T,
			decltype(std::declval< T >().reserve(std::size_t{})) >
		{
			void operator()(T& c, std::size_t s)const{
				c.reserve(s);
			}
		};


	}


	template < typename T >
	struct to_shared_const_buffer_t{
		static_assert(
			detail::buffers_constructible< T >::value,
			"boost::asio::buffer must be callable with an object of type T"
			"alternatively you can specialize to_shared_const_buffer_t with a "
			"function operator that takes an object of type T and "
			"returns a webservice::shared_const_buffer");

		shared_const_buffer operator()(T data)const{
			return shared_const_buffer(std::move(data));
		}
	};


	template < typename T >
	struct from_multi_buffer_t{
		static_assert(
			std::is_default_constructible< T >::value &&
			detail::has_insert_function< T >::value,
			"T must be default constructable, have a type member value_type "
			"with sizeof equal 1 as well as the iterator functions .end() and "
			".insert(iterator, const value_type*, const value_type*); "
			"alternatively you can specialize from_multi_buffer_t with a "
			"function operator that takes a boost::beast::multi_buffer object "
			"returns an object of type T");

		T operator()(boost::beast::multi_buffer const& buffer)const{
			T result;

			detail::call_reserve< T > reserve;
			reserve(result, boost::asio::buffer_size(buffer.data()));

			auto const end = boost::asio::buffer_sequence_end(buffer.data());
			for(
				auto iter = boost::asio::buffer_sequence_begin(buffer.data());
				iter != end; ++iter
			){
				boost::asio::const_buffer buffer(*iter);
				auto const data = reinterpret_cast<
					typename T::value_type const* >(buffer.data());
				auto const size = buffer.size();
				result.insert(result.end(), data, data + size);
			}

			return result;
		}
	};


}


#endif
