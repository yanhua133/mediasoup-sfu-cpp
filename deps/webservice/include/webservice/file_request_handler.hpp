//-----------------------------------------------------------------------------
// Copyright (c) 2018 Benjamin Buch
//
// https://github.com/bebuch/webservice
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _webservice__file_request_handler__hpp_INCLUDED_
#define _webservice__file_request_handler__hpp_INCLUDED_

#include "http_request_handler.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>


namespace webservice{


	class file_request_handler: public http_request_handler{
	public:
		file_request_handler(std::string doc_root)
			: doc_root_(std::move(doc_root)) {}

		void operator()(http_request&& req, http_response&& send)override;


		/// \brief Get HTTP document root directory
		std::string const& doc_root()const;

		/// \brief Concat relative path with document root directory
		///
		/// This also adds index.html if the target ended on slash.
		std::string with_path(boost::beast::string_view target)const;


	protected:
		virtual void on_file_not_found(
			http_request&& req,
			http_response&& send
		);

		/// \brief Make sure we can handle the method
		static bool is_unknown_http_method(http_request const& req);

		/// \brief Request path must be absolute and not contain "..".
		static bool is_illegal_request_target(http_request const& req);

		/// \brief Send the given file body as answer
		static void send_body(
			http_request&& req,
			http_response&& send,
			boost::beast::http::file_body::value_type&& body,
			boost::beast::string_view mime_type
		);


	private:
		std::string doc_root_;
	};


}


#endif
