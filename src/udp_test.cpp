#include <yaskawa-ethernet/udp/client.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <iostream>

dr::yaskawa::udp::Client * client;
int read_count = 0;
boost::asio::steady_timer * timer;

void onTimeout(boost::system::error_code const & error) {
	if (error) throw boost::system::system_error(error);
	std::cout << "Reading at " << read_count << " Hz.\n";
	read_count = 0;
	timer->expires_from_now(std::chrono::seconds(1));
	timer->async_wait(onTimeout);
}

void onReadByte(dr::yaskawa::ErrorOr<dr::yaskawa::ReadInt8Variable::Response> const & response) {
	auto error = response.error();
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.detailed_message() << "\n";
	} else {
		std::cout << "Read byte variable with value " << int(response.get().value) << "\n";
	}
	++read_count;
	client->readByteVariable(0, 1500, onReadByte);
}

void onConnect(boost::system::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected to " << client->socket().remote_endpoint() << " .\n";
	client->readByteVariable(0, 1500, onReadByte);
	timer->async_wait(onTimeout);
}

int main(int argc, char * * argv) {
	boost::asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);
	::client = &client;
	boost::asio::steady_timer timer(ios);
	::timer = &timer;

	std::string host = "10.0.0.2";
	std::string port = "10040";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	client.connect(host, port, 1500, onConnect);
	ios.run();
}
