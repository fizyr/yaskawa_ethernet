#include "../include/yaskawa_ethernet/tcp/client.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

dr::yaskawa::tcp::Client * client;
int read_count = 0;
boost::asio::steady_timer * timer;

void onTimeout(boost::system::error_code const & error) {
	if (error) throw boost::system::system_error(error);
	std::cout << "Reading at " << read_count << " Hz.\n";
	read_count = 0;
	timer->expires_from_now(std::chrono::seconds(1));
	timer->async_wait(onTimeout);
}

void onReadByte(dr::ErrorOr<std::uint8_t> response) {
	if (!response) {
		std::cout << response.error().fullMessage();
		return;
	}

	++read_count;
	std::cout << "Read byte variable with value " << int(*response) << "\n";
	client->readByteVariable(1, onReadByte);
}

void onStart(dr::ErrorOr<std::string> response) {
	if (!response) {
		std::cout << response.error().fullMessage();
		return;
	}

	std::cout << "Start request succeeded: " << *response << "\n";
	client->readByteVariable(1, onReadByte);
	timer->async_wait(onTimeout);
}

void onConnect(std::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected.\n";
	client->start(-1, onStart);
}

int main(int argc, char * * argv) {
	boost::asio::io_service ios;
	dr::yaskawa::tcp::Client client(ios);
	::client = &client;
	boost::asio::steady_timer timer(ios);
	::timer = &timer;

	std::string host = "10.0.0.2";
	std::string port = "80";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	client.connect(host, port, 1500ms, onConnect);
	ios.run();
}
