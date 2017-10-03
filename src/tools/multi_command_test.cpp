#include "udp/client.hpp"
#include "udp/client.hpp"

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
using namespace dr::yaskawa;

dr::yaskawa::udp::Client * client;
int command_count = 0;
asio::steady_timer * timer;

std::uint8_t byte_value    = 0;
std::int16_t int16_value   = -5;
std::int32_t int32_value   = -5;
float        float32_value = -5;

std::chrono::milliseconds timeout = 200ms;

void write();
void read();

template<typename T>
void checkValue(std::string const & name, T a, T b) {
	if (a != b) std::cerr << "Value mismatch for " << name << ": read " << a << ", expected " << b << "\n";
}

void onTimeout(std::error_code const & error) {
	if (error) throw std::system_error(error);
	std::cout << "Executing commands at " << command_count << " Hz.\n";
	command_count = 0;
	timer->expires_from_now(std::chrono::seconds(1));
	timer->async_wait(onTimeout);
}

void write() {
	client->sendCommands(
		std::make_tuple(WriteUint8Var{5, byte_value}, WriteInt16Var{6, int16_value}, WriteInt32Var{7, int32_value}, WriteFloat32Var{8, float32_value}),
		timeout,
		[] (dr::ErrorOr<std::tuple<Empty, Empty, Empty, Empty>> result) {
			if (!result) {
				std::cerr << "Failed to write: " << result.error().fullMessage() << "\n";
			} else {
				++command_count;
			}
			read();
		}
	);
}

void read() {
	client->sendCommands(
		std::make_tuple(ReadUint8Var{5}, ReadInt16Var{6}, ReadInt32Var{7}, ReadFloat32Var{8}),
		timeout,
		[] (dr::ErrorOr<std::tuple<std::uint8_t, std::int16_t, std::int32_t, float>> result) {
			if (!result) {
				std::cerr << "Failed to read: " << result.error().fullMessage() << "\n";
			} else {
				std::uint8_t int8    = std::get<0>(*result);
				std::int16_t int16   = std::get<1>(*result);
				std::int32_t int32   = std::get<2>(*result);
				float        float32 = std::get<3>(*result);
				checkValue("int8", int8, byte_value);
				checkValue("int16", int16, int16_value);
				checkValue("int32", int32, int32_value);
				checkValue("float32", float32, float32_value);
				++byte_value;
				++int16_value;
				++int32_value;
				++float32_value;
				++command_count;
			}
			write();
		}
	);
}


void onConnect(std::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected to " << client->socket().remote_endpoint() << ".\n";
	write();
	timer->async_wait(onTimeout);
}

int main(int argc, char * * argv) {
	asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);
	::client = &client;
	asio::steady_timer timer(ios);
	::timer = &timer;

	client.on_error = [] (dr::DetailedError const & error) {
		std::cout << "Communication error: " << error.fullMessage() << "\n";
	};

	std::string host = "10.0.0.2";
	std::string port = "10040";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	client.connect(host, port, 1500ms, onConnect);
	ios.run();
}
