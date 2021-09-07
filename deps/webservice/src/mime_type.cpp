//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <webservice/mime_type.hpp>


namespace webservice{


	boost::beast::string_view mime_type(boost::beast::string_view path){
		using boost::beast::string_view;
		using boost::beast::iless;
		static std::map< string_view, string_view, iless > const map{
				{".htm" , "text/html"},
				{".html", "text/html"},
				{".php" , "text/html"},
				{".css" , "text/css"},
				{".txt" , "text/plain"},
				{".js"  , "application/javascript"},
				{".json", "application/json"},
				{".xml" , "application/xml"},
				{".swf" , "application/x-shockwave-flash"},
				{".flv" , "video/x-flv"},
				{".png" , "image/png"},
				{".jpe" , "image/jpeg"},
				{".jpeg", "image/jpeg"},
				{".jpg" , "image/jpeg"},
				{".gif" , "image/gif"},
				{".bmp" , "image/bmp"},
				{".ico" , "image/vnd.microsoft.icon"},
				{".tiff", "image/tiff"},
				{".tif" , "image/tiff"},
				{".svg" , "image/svg+xml"},
				{".svgz", "image/svg+xml"}
			};

		auto const ext = [&path]{
				auto const pos = path.rfind(".");
				return pos != string_view::npos
					? path.substr(pos) : string_view{};
			}();

		auto iter = map.find(ext);
		if(iter != map.end()){
			return iter->second;
		}else{
			return "application/text";
		}
	}


}
