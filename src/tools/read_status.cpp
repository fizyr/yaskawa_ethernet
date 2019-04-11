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

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
using namespace dr::yaskawa;

std::chrono::milliseconds timeout = 200ms;

void readStatus(udp::Client & client) {
	client.sendCommand(ReadStatus{}, timeout, [&client] (Result<dr::yaskawa::Status> const & result) {
		if (!result) {
			std::cout << "Error reading status: " << result.error().format() << "\n";
			client.close();
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
			<< "servo_on:           " << (result->servo_on           ? "true" : "false") << "\n";

		readStatus(client);
	});
}

void connect(udp::Client & client, std::string host, std::string port) {
	client.connect(host, port, timeout, [&client] (Error error) {
		if (error) {
			std::cout << error.format() << "\n";
			client.close();
			return;
		}
		std::cout << "Connected to " << client.socket().remote_endpoint() << ".\n";
		readStatus(client);
	});
}

int main(int argc, char * * argv) {
	asio::io_service ios;
	dr::yaskawa::udp::Client client(ios);

	client.on_error = [&client] (Error const & error) {
		std::cout << "Communication error: " << error.format() << "\n";
		client.close();
	};

	std::string host = "10.0.0.2";
	std::string port = "10040";

	if (argc > 1) host = argv[1];
	if (argc > 2) port = argv[2];

	connect(client, host, port);
	ios.run();
}
