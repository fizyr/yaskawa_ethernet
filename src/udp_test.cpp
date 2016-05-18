#include "udp/client.hpp"

#include <boost/asio/io_service.hpp>

#include <iostream>

dr::yaskawa::udp::Client * client;

void onReadByte(dr::yaskawa::ErrorOr<dr::yaskawa::ReadInt8Variable::Response> const & response) {
	auto error = response.error();
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.detailed_message() << "\n";
		return;
	}

	std::cout << "Read byte variable with value " << int(response.get().value) << "\n";
}

void onConnect(boost::system::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected to " << client->socket().remote_endpoint() << " .\n";
	client->readByteVariable({1}, 1500, onReadByte);
}

int main() {
	boost::asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);
	::client = &client;
	client.connect("10.0.0.2", 10040, 1500, onConnect);
	ios.run();
}
