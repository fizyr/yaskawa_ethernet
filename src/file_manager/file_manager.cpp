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

#include <iostream>
#include <string>

using namespace std::string_literals;
using namespace std::chrono_literals;

using namespace dr::yaskawa;

void usage(char const * name) {
	std::cerr
		<< "usage: " << name << "host command [args...]\n\n"
		<< "commands:\n"
		<< "\tls [type]\n"
		<< "\tget name\n"
		<< "\tput name\n"
		<< "\tdelete name\n";
}

struct Options {
	std::string host;
	std::string command;
	std::vector<std::string> args;
};

void executeCommand(dr::yaskawa::udp::Client & client, Options const & options) {
	if (options.command == "ls") {
		client.readFileList(options.args[0], 100ms, [] (Result<std::vector<std::string>> result) {
			if (!result) {
				std::cerr << "Failed to read file list: " << result.error().format() << "\n";
				std::exit(2);
			}
			for (std::string const & file : *result) {
				std::cout << file << "\n";
			}
		}, nullptr);
	} else if (options.command == "get") {
		client.readFile(options.args[0], 3s, [] (Result<std::string> result) {
			if (!result) {
				std::cerr << "Failed to read file: " << result.error().format() << "\n";
				std::exit(2);
			}
			std::cout << *result;
		}, nullptr);
	} else if (options.command == "put") {
		std::stringstream data;
		data << std::cin.rdbuf();
		client.writeFile(options.args[0], data.str(), 3s, [] (Result<void> result) {
			if (!result) {
				std::cerr << "Failed to write file: " << result.error().format() << "\n";
				std::exit(2);
			}
		}, nullptr);
	} else if (options.command == "delete") {
		client.deleteFile(options.args[0], 3s, [] (Result<void> result) {
			if (!result) {
				std::cerr << "Failed to delete file: " << result.error().format() << "\n";
				std::exit(2);
			}
		});
	} else {
		std::cerr << "unknown command: " << options.command << "\n";
	}
}

int main(int argc, char * * argv) {
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	Options options;
	options.host    = argv[1];
	options.command = argv[2];

	if (options.command == "ls") {
		if (argc > 4) {
			std::cerr << "ls command takes one argument at most\n";
			return 1;
		}
		options.args.push_back(argc > 3 ? argv[3] : "*.*");
	} else if (options.command == "get") {
		if (argc != 4) {
			std::cerr << "get command takes exactly one argument\n";
			return 1;
		}
		options.args.push_back(argv[3]);
	} else if (options.command == "put") {
		if (argc != 4) {
			std::cerr << "put command takes exactly one argument\n";
			return 1;
		}
		options.args.push_back(argv[3]);
	} else {
		std::cerr << "unknown command: " << options.command << "\n";
	}


	asio::io_service ios;
	dr::yaskawa::udp::Client client{ios};
	client.connect(argv[1], 10040, 100ms, [&client, &options] (Error error) {
		if (error) {
			std::cerr << "Failed to connect to " << options.host << ":10040: " << error.format() << "\n";
			std::exit(1);
		}

		executeCommand(client, options);
	});

	ios.run();
	return 0;
}
