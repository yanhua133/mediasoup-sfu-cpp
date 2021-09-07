//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/shared_const_buffer.hpp>

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

struct bool_{ bool v; };

std::ostream& operator<<(std::ostream& os, bool_ v){
	if(v.v){
		os << "\033[1;32m";
	}else{
		os << "\033[1;31m";
	}
	os.operator<<(v.v);
	os << "\033[0m";
	return os;
}


int main(){
	std::cout << std::boolalpha;

	{
		std::vector< std::uint8_t > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(data);
		std::cout << "vector< uint8_t >: "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< std::uint8_t const* >(buffer.begin()->data()),
				static_cast< std::uint8_t const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::vector< char > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(data);
		std::cout << "vector< char >: "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::string data("12345");
		webservice::shared_const_buffer buffer(data);
		std::cout << "string: "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		auto data = std::make_shared< std::string >("12345");
		webservice::shared_const_buffer buffer(data);
		std::cout << "shared_ptr< string >: "
			<< bool_{std::equal(data->begin(), data->end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		auto data = std::make_shared< std::string const >("12345");
		webservice::shared_const_buffer buffer(data);
		std::cout << "shared_ptr< string const >: "
			<< bool_{std::equal(data->begin(), data->end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::vector< std::uint8_t > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout << "vector< uint8_t >, created from data() and size(): "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< std::uint8_t const* >(buffer.begin()->data()),
				static_cast< std::uint8_t const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::vector< char > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout << "vector< char >, created from data() and size(): "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::string data("12345");
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout << "string, created from data() and size(): "
			<< bool_{std::equal(data.begin(), data.end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		auto data = std::make_shared< std::string >("12345");
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout << "shared_ptr< string >, created from data() and size(): "
			<< bool_{std::equal(data->begin(), data->end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		auto data = std::make_shared< std::string const >("12345");
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout
			<< "shared_ptr< string const >, created from data() and size(): "
			<< bool_{std::equal(data->begin(), data->end(),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::vector< int > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(data);
		std::cout << "vector< int >: "
			<< bool_{std::equal(
				reinterpret_cast< char const* >(data.data()),
				reinterpret_cast< char const* >(data.data())
					+ data.size() * sizeof(int),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}

	{
		std::vector< int > data{1, 2, 3, 4, 5};
		webservice::shared_const_buffer buffer(
			data, webservice::construct_via_data_and_size);
		std::cout << "vector< int >, created from data() and size(): "
			<< bool_{std::equal(
				reinterpret_cast< char const* >(data.data()),
				reinterpret_cast< char const* >(data.data())
					+ data.size() * sizeof(int),
				static_cast< char const* >(buffer.begin()->data()),
				static_cast< char const* >(buffer.begin()->data())
					+ buffer.begin()->size())} << '\n';
	}
}
