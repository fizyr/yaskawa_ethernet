#include "../include/yaskawa_ethernet/udp/client.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
using namespace dr::yaskawa;

dr::yaskawa::udp::Client * client;
int command_count = 0;
boost::asio::steady_timer * timer;

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

void onWriteByte(dr::ErrorOr<void> result);
void onReadByte(dr::ErrorOr<std::uint8_t> result);
void onWriteInt16(dr::ErrorOr<void> result);
void onReadInt16(dr::ErrorOr<std::int16_t> result);
void onWriteInt32(dr::ErrorOr<void> result);
void onReadInt32(dr::ErrorOr<std::int32_t> result);
void onWriteFloat(dr::ErrorOr<void> result);
void onReadFloat(dr::ErrorOr<float> result);
void onWritePosition(dr::ErrorOr<void> result);
void onReadPosition(dr::ErrorOr<Position> const & result);

void onTimeout(boost::system::error_code const & error) {
	if (error) throw boost::system::system_error(error);
	std::cout << "Executing commands at " << command_count << " Hz.\n";
	command_count = 0;
	timer->expires_from_now(std::chrono::seconds(1));
	timer->async_wait(onTimeout);
}

void onWriteByte(dr::ErrorOr<void> result) {
	if (!result) {
		std::cerr << "Failed to write byte: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
	}
	client->readByte(5, timeout, onReadByte);
}

void onReadByte(dr::ErrorOr<std::uint8_t> result) {
	if (!result) {
		std::cout << "Failed to read byte: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
		if (*result != byte_value) std::cout << "Read wrong byte value: " << int(*result) << ", expected " << byte_value << ".\n";
		++byte_value;
	}
	client->writeByte(5, byte_value, timeout, onWriteByte);
}

void onWriteInt16(dr::ErrorOr<void> result) {
	if (!result) {
		std::cerr << "Failed to write int16: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
	}
	client->readInt16(6, timeout, onReadInt16);
}

void onReadInt16(dr::ErrorOr<std::int16_t> result) {
	if (!result) {
		std::cout << "Failed to read int16: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
		if (*result != int16_value) std::cout << "Read wrong int16 value: " << int(*result) << ", expected " << int16_value << ".\n";
		++int16_value;
	}
	client->writeInt16(6, int16_value, timeout, onWriteInt16);
}

void onWriteInt32(dr::ErrorOr<void> result) {
	if (!result) {
		std::cerr << "Failed to write int32: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
	}
	client->readInt32(7, timeout, onReadInt32);
}

void onReadInt32(dr::ErrorOr<std::int32_t> result) {
	if (!result) {
		std::cout << "Failed to read int32: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
		if (*result != int32_value) std::cout << "Read wrong int32 value: " << int(*result) << ", expected " << int32_value << ".\n";
		++int32_value;
	}
	client->writeInt32(7, int32_value, timeout, onWriteInt32);
}

void onWriteFloat(dr::ErrorOr<void> result) {
	if (!result) {
		std::cerr << "Failed to write float32 value: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
	}
	client->readFloat32(8, timeout, onReadFloat);
}

void onReadFloat(dr::ErrorOr<float> result) {
	if (!result) {
		std::cout << "Failed to read float32: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
		if (*result != float32_value) std::cout << "Read wrong float32 value: " << (*result) << ", expected " << float32_value << ".\n";
		++float32_value;
	}
	client->writeFloat32(8, float32_value, timeout, onWriteFloat);
}

void onWritePosition(dr::ErrorOr<void> result) {
	if (!result) {
		std::cerr << "Failed to write position value: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
	}
	client->readRobotPosition(9, timeout, onReadPosition);
}

void onReadPosition(dr::ErrorOr<Position> const & result) {
	if (!result) {
		std::cout << "Failed to read position: " << result.error().fullMessage() << "\n";
	} else {
		++command_count;
		if (*result != positions[position_index]) {
			std::cout << "Read wrong position\n"
				<< "got:      " << *result << "\n"
				<< "expected: " << positions[position_index] << "\n";
		}
	}
	position_index = (position_index + 1) % positions.size();
	client->writeRobotPosition(9, positions[position_index], timeout, onWritePosition);
}

void onConnect(std::error_code const & error) {
	if (error) {
		std::cout << "Error " << error.category().name() << ":" << error.value() << ": " << error.message() << "\n";
		return;
	}
	std::cout << "Connected to " << client->socket().remote_endpoint() << ".\n";
	client->writeByte(5, byte_value, timeout, onWriteByte);
	client->writeInt16(6, int16_value, timeout, onWriteInt16);
	client->writeInt32(7, int32_value, timeout, onWriteInt32);
	client->writeFloat32(8, float32_value, timeout, onWriteFloat);
	client->writeRobotPosition(9, positions[position_index], timeout, onWritePosition);
	timer->async_wait(onTimeout);
}

int main(int argc, char * * argv) {
	boost::asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);
	::client = &client;
	boost::asio::steady_timer timer(ios);
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
