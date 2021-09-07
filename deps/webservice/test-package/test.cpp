#include <webservice/file_request_handler.hpp>
#include <webservice/server.hpp>

int main(){
	using webservice::file_request_handler;
	using webservice::server;
	server server(
		std::make_unique< file_request_handler >("./"),
		nullptr, nullptr,
		boost::asio::ip::make_address("127.0.0.1"), 56789, 1);
	(void)server;
}
