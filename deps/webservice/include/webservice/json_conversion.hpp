//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__json_conversion__hpp_INCLUDED_
#define _webservice__json_conversion__hpp_INCLUDED_

#include "conversion.hpp"

#include <nlohmann/json.hpp>


namespace webservice{


	template <>
	struct to_shared_const_buffer_t< nlohmann::json >{
		shared_const_buffer operator()(nlohmann::json const& data)const{
			try{
				return shared_const_buffer(data.dump());
			}catch(nlohmann::json::exception const& e){
				throw std::runtime_error(std::string(e.what())
					+ "; dump failed");
			}
		}
	};

	template <>
	struct from_multi_buffer_t< nlohmann::json >{
		nlohmann::json operator()(
			boost::beast::multi_buffer const& buffer
		)const{
			constexpr from_multi_buffer_t< std::string > to_string;
			auto data = to_string(buffer);
			try{
				return nlohmann::json::parse(data);
			}catch(nlohmann::json::exception const& e){
				throw std::runtime_error(std::string(e.what())
					+ "; parsed expression '" + data + "'");
			}
		}
	};


}


#endif
