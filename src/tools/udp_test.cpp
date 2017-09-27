#include "../include/yaskawa_ethernet/udp/client.hpp"
#include "../include/yaskawa_ethernet/udp/client.hpp"

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

std::array<Position, 4> positions {{
	PulsePosition{std::array<int, 8>{{0, 1, 2, 3, 4,   5, 0, 0}}, 1},
	PulsePosition{std::array<int, 8>{{6, 7, 8, 9, 10, 11, 0, 0}}, 2},
	CartesianPosition{std::array<double, 6>{{12, 13, 14, 15, 16, 17}}, CoordinateSystem::base,  0xff, 3},
	CartesianPosition{std::array<double, 6>{{18, 19, 20, 21, 22, 23}}, CoordinateSystem::user3, 0x00, 4},
}};

int position_index = 0;

void writeByte();
void readByte();
void writeInt16();
void readInt16();
void writeInt32();
void readInt32();
void writeFloat32();
void readFloat32();
void readPosition();
void writePosition();

void onTimeout(std::error_code const & error) {
	if (error) throw std::system_error(error);
	std::cout << "Executing commands at " << command_count << " Hz.\n";
	command_count = 0;
	timer->expires_from_now(std::chrono::seconds(1));
	timer->async_wait(onTimeout);
}

void writeByte() {
	client->sendCommand(WriteInt8Variable{5, {byte_value}}, timeout, [] (dr::ErrorOr<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write byte: " << result.error().fullMessage() << "\n";
		} else {
			++command_count;
		}
		readByte();
	});
}

void readByte() {
	client->sendCommand(ReadInt8Variable{5, 1}, timeout, [] (dr::ErrorOr<std::vector<std::uint8_t>> const & result) {
		if (!result) {
			std::cout << "Failed to read byte: " << result.error().fullMessage() << "\n";
		} else {
			int value = (*result)[0];
			++command_count;
			if (value != byte_value) std::cout << "Read wrong byte value: " << value << ", expected " << byte_value << ".\n";
			++byte_value;
		}
		writeByte();
	});
}

void writeInt16() {
	client->sendCommand(WriteInt16Variable{6, {int16_value}}, timeout, [] (dr::ErrorOr<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write int16: " << result.error().fullMessage() << "\n";
		} else {
			++command_count;
		}
		readInt16();
	});
}

void readInt16() {
	client->sendCommand(ReadInt16Variable{6, 1}, timeout, [] (dr::ErrorOr<std::vector<std::int16_t>> result) {
		if (!result) {
			std::cout << "Failed to read int16: " << result.error().fullMessage() << "\n";
		} else {
			int value = (*result)[0];
			++command_count;
			if (value != int16_value) std::cout << "Read wrong int16 value: " << value << ", expected " << int16_value << ".\n";
			++int16_value;
		}
		writeInt16();
	});
}

void writeInt32() {
	client->sendCommand(WriteInt32Variable{7, {int32_value}}, timeout, [] (dr::ErrorOr<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write int32: " << result.error().fullMessage() << "\n";
		} else {
			++command_count;
		}
		readInt32();
	});
}

void readInt32() {
	client->sendCommand(ReadInt32Variable{7, 1}, timeout, [] (dr::ErrorOr<std::vector<std::int32_t>> result) {
		if (!result) {
			std::cout << "Failed to read int32: " << result.error().fullMessage() << "\n";
		} else {
			int value = (*result)[0];
			++command_count;
			if (value != int32_value) std::cout << "Read wrong int32 value: " << int(value) << ", expected " << int32_value << ".\n";
			++int32_value;
		}
		writeInt32();
	});
}

void writeFloat32() {
	client->sendCommand(WriteFloat32Variable{8, {float32_value}}, timeout, [] (dr::ErrorOr<void> result) {
		if (!result) {
			std::cerr << "Failed to write float32 value: " << result.error().fullMessage() << "\n";
		} else {
			++command_count;
		}
	});
}

void readFloat32() {
	client->sendCommand(ReadFloat32Variable{8, 1}, timeout, [] (dr::ErrorOr<std::vector<float>> result) {
		if (!result) {
			std::cout << "Failed to read float32: " << result.error().fullMessage() << "\n";
		} else {
			float value = (*result)[0];
			++command_count;
			if (value != float32_value) std::cout << "Read wrong float32 value: " << value << ", expected " << float32_value << ".\n";
			++float32_value;
		}
	});
}

void writePosition() {
	client->sendCommand(WritePositionVariable{9, {positions[position_index]}}, timeout, [] (dr::ErrorOr<void> result) {
		if (!result) {
			std::cerr << "Failed to write position value: " << result.error().fullMessage() << "\n";
		} else {
			++command_count;
		}
	});
}

void readPosition() {
	client->sendCommand(ReadPositionVariable{9, 1}, timeout, [] (dr::ErrorOr<std::vector<Position>> const & result) {
		if (!result) {
			std::cout << "Failed to read position: " << result.error().fullMessage() << "\n";
		} else {
			Position value = (*result)[0];
			++command_count;
			if (value != positions[position_index]) {
				std::cout << "Read wrong position\n"
					<< "got:      " << value << "\n"
					<< "expected: " << positions[position_index] << "\n";
			}
		}
		position_index = (position_index + 1) % positions.size();
	});
}

void onConnect(std::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected to " << client->socket().remote_endpoint() << ".\n";
	writeByte();
	writeInt16();
	writeInt32();
	writeFloat32();
	writePosition();
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
