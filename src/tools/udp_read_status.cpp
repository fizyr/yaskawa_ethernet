#include "../include/yaskawa_ethernet/udp/client.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
using namespace dr::yaskawa;

std::chrono::milliseconds timeout = 200ms;

void readStatus(dr::yaskawa::udp::Client & client) {
	client.readStatus(timeout, [&client] (dr::ErrorOr<dr::yaskawa::Status> const & result) {
		if (!result) {
			std::cout << "Error reading status: " << result.error().category().name() << ":" << result.error().value() << ": " << result.error().message() << "\n";
			return;
		}
		std::cout
			<< "----------\n"
			<< "step:               " << (result->step               ? "true" : "false") << "\n"
			<< "one_cycle:          " << (result->one_cycle          ? "true" : "false") << "\n"
			<< "continuous:         " << (result->continuous         ? "true" : "false") << "\n"
			<< "running:            " << (result->running            ? "true" : "false") << "\n"
			<< "speed_limited:      " << (result->speed_limited      ? "true" : "false") << "\n"
			<< "teach:              " << (result->teach              ? "true" : "false") << "\n"
			<< "play:               " << (result->play               ? "true" : "false") << "\n"
			<< "remote:             " << (result->remote             ? "true" : "false") << "\n"
			<< "teach_pendant_hold: " << (result->teach_pendant_hold ? "true" : "false") << "\n"
			<< "external_hold:      " << (result->external_hold      ? "true" : "false") << "\n"
			<< "command_hold:       " << (result->command_hold       ? "true" : "false") << "\n"
			<< "alarm:              " << (result->alarm              ? "true" : "false") << "\n"
			<< "error:              " << (result->error              ? "true" : "false") << "\n"
			<< "servo_on:           " << (result->servo_on           ? "true" : "false") << "\n"
			<< "----------\n";

		readStatus(client);
	});
}

void connect(dr::yaskawa::udp::Client & client, std::string host, std::string port) {
	client.connect(host, port, timeout, [&client] (dr::DetailedError error) {
		if (error) {
			std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
			return;
		}
		std::cout << "Connected to " << client.socket().remote_endpoint() << ".\n";
		readStatus(client);
	});
}

int main(int argc, char * * argv) {
	boost::asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);

	client.on_error = [] (dr::DetailedError const & error) {
		std::cout << "Communication error: " << error.fullMessage() << "\n";
	};


	std::string host = "10.0.0.2";
	std::string port = "10040";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	connect(client, host, port);
	ios.run();
}
