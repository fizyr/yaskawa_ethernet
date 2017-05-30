#include <iostream>
#include <string>

#include <yaskawa_ethernet/udp/client.hpp>

using namespace std::string_literals;
using namespace std::chrono_literals;

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
		client.readFileList(options.args[0], 100ms, [&options] (dr::ErrorOr<std::vector<std::string>> result) {
			if (!result) {
				std::cerr << "Failed to read file list: " << result.error().fullMessage() << "\n";
				std::exit(2);
			}
			for (std::string const & file : *result) {
				std::cout << file << "\n";
			}
		});
	} else if (options.command == "get") {
		client.readFile(options.args[0], 3s, [&options] (dr::ErrorOr<std::string> result) {
			if (!result) {
				std::cerr << "Failed to read file: " << result.error().fullMessage() << "\n";
				std::exit(2);
			}
			std::cout << *result;
		}, nullptr);
	} else if (options.command == "put") {
		std::stringstream data;
		data << std::cin.rdbuf();
		client.writeFile(options.args[0], data.str(), 3s, [&options] (dr::ErrorOr<void> result) {
			if (!result) {
				std::cerr << "Failed to write file: " << result.error().fullMessage() << "\n";
				std::exit(2);
			}
		}, nullptr);
	} else if (options.command == "delete") {
		client.deleteFile(options.args[0], 3s, [&options] (dr::ErrorOr<void> result) {
			if (!result) {
				std::cerr << "Failed to delete file: " << result.error().fullMessage() << "\n";
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


	boost::asio::io_service ios;
	dr::yaskawa::udp::Client client{ios};
	client.connect(argv[1], 10040, 100ms, [&client, &options] (std::error_code error) {
		if (error) {
			std::cerr << "Failed to connect to " << options.host << ":10040: " << error.message() << "\n";
			std::exit(1);
		}

		executeCommand(client, options);
	});

	ios.run();
	return 0;
}
