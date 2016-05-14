#include "client.hpp"

#include <boost/asio/io_service.hpp>

#include <iostream>

dr::yaskawa::EthernetClient * client;

void onStart(dr::yaskawa::ErrorOr<std::string> const & response) {
	auto error = response.errorDetails();
	if (error.code) {
		std::cout << "Error " << error.code.category().name() << ":" << error.code.value() << ": " << error.code.message() << ": " << error.message << "\n";
		return;
	}

	std::cout << "Start request succeeded: " << response.get() << "\n";
}

void onConnect(boost::system::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected.\n";
	client->start(-1, onStart);
}

int main() {
	boost::asio::io_service ios;
	dr::yaskawa::EthernetClient client(ios);
	::client = &client;
	client.connect("localhost", "1080", onConnect);
	ios.run();
}
