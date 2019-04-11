/* Copyright 2016-2019 Fizyr B.V. - https://fizyr.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
	client->sendCommand(WriteUint8Var{5, byte_value}, timeout, [] (Result<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write byte: " << result.error().format() << "\n";
		} else {
			++command_count;
		}
		readByte();
	});
}

void readByte() {
	client->sendCommand(ReadUint8Var{5}, timeout, [] (Result<std::uint8_t> const & result) {
		if (!result) {
			std::cout << "Failed to read byte: " << result.error().format() << "\n";
		} else {
			int value = (*result);
			++command_count;
			if (value != byte_value) std::cout << "Read wrong byte value: " << value << ", expected " << byte_value << ".\n";
			++byte_value;
		}
		writeByte();
	});
}

void writeInt16() {
	client->sendCommand(WriteInt16Var{6, int16_value}, timeout, [] (Result<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write int16: " << result.error().format() << "\n";
		} else {
			++command_count;
		}
		readInt16();
	});
}

void readInt16() {
	client->sendCommand(ReadInt16Var{6}, timeout, [] (Result<std::int16_t> result) {
		if (!result) {
			std::cout << "Failed to read int16: " << result.error().format() << "\n";
		} else {
			int value = (*result);
			++command_count;
			if (value != int16_value) std::cout << "Read wrong int16 value: " << value << ", expected " << int16_value << ".\n";
			++int16_value;
		}
		writeInt16();
	});
}

void writeInt32() {
	client->sendCommand(WriteInt32Var{7, int32_value}, timeout, [] (Result<void> const & result) {
		if (!result) {
			std::cerr << "Failed to write int32: " << result.error().format() << "\n";
		} else {
			++command_count;
		}
		readInt32();
	});
}

void readInt32() {
	client->sendCommand(ReadInt32Var{7}, timeout, [] (Result<std::int32_t> result) {
		if (!result) {
			std::cout << "Failed to read int32: " << result.error().format() << "\n";
		} else {
			int value = (*result);
			++command_count;
			if (value != int32_value) std::cout << "Read wrong int32 value: " << int(value) << ", expected " << int32_value << ".\n";
			++int32_value;
		}
		writeInt32();
	});
}

void writeFloat32() {
	client->sendCommand(WriteFloat32Var{8, float32_value}, timeout, [] (Result<void> result) {
		if (!result) {
			std::cerr << "Failed to write float32 value: " << result.error().format() << "\n";
		} else {
			++command_count;
		}
	});
}

void readFloat32() {
	client->sendCommand(ReadFloat32Var{8}, timeout, [] (Result<float> result) {
		if (!result) {
			std::cout << "Failed to read float32: " << result.error().format() << "\n";
		} else {
			float value = (*result);
			++command_count;
			if (value != float32_value) std::cout << "Read wrong float32 value: " << value << ", expected " << float32_value << ".\n";
			++float32_value;
		}
	});
}

void writePosition() {
	client->sendCommand(WritePositionVar{9, positions[position_index]}, timeout, [] (Result<void> result) {
		if (!result) {
			std::cerr << "Failed to write position value: " << result.error().format() << "\n";
		} else {
			++command_count;
		}
	});
}

void readPosition() {
	client->sendCommand(ReadPositionVar{9}, timeout, [] (Result<Position> const & result) {
		if (!result) {
			std::cout << "Failed to read position: " << result.error().format() << "\n";
		} else {
			Position value = (*result);
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

void onConnect(Error error) {
	if (error) {
		std::cout << error.format() << "\n";
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

	client.on_error = [] (Error const & error) {
		std::cout << "Communication error: " << error.format() << "\n";
	};

	std::string host = "10.0.0.2";
	std::string port = "10040";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	client.connect(host, port, 1500ms, onConnect);
	ios.run();
}
