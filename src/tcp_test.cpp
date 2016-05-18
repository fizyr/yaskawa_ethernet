#include "tcp/client.hpp"

#include <boost/asio/io_service.hpp>

#include <iostream>

dr::yaskawa::tcp::Client * client;

void onReadByte(dr::yaskawa::ErrorOr<std::uint8_t> response) {
	auto error = response.error();
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.detailed_message() << "\n";
		return;
	}

	std::cout << "Read byte variable with value " << int(response.get()) << "\n";
}

void onStart(dr::yaskawa::ErrorOr<dr::yaskawa::tcp::CommandResponse> response) {
	auto error = response.error();
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.detailed_message() << "\n";
		return;
	}

	std::cout << "Start request succeeded: " << response.get().message << "\n";
	client->sendCommand<dr::yaskawa::ReadInt8Variable>({1}, onReadByte);
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
	dr::yaskawa::tcp::Client client(ios);
	::client = &client;
	client.connect("10.0.0.2", 80, 1500, onConnect);
	ios.run();
}
