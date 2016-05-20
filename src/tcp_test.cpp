#include <yaskawa-ethernet/tcp/client.hpp>

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

int main(int argc, char * * argv) {
	boost::asio::io_service ios;
	dr::yaskawa::tcp::Client client(ios);
	::client = &client;

	std::string host = "10.0.0.2";
	std::string port = "80";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	client.connect(host, port, 1500, onConnect);
	ios.run();
}
