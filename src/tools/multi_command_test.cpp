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
		[] (Result<std::tuple<Empty, Empty, Empty, Empty>> result) {
			if (!result) {
				std::cerr << "Failed to write: " << result.error().format() << "\n";
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
		[] (Result<std::tuple<std::uint8_t, std::int16_t, std::int32_t, float>> result) {
			if (!result) {
				std::cerr << "Failed to read: " << result.error().format() << "\n";
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


void onConnect(Error const & error) {
	if (error) {
		std::cout << error.format() << "\n";
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
