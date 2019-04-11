# Overview

This library contains an asynchronous client for the UDP based Yaskawa High Speed Ethernet Server functionality.

# Example

The following example is a program that reads and prints the D001 variable for a controller on IP address `10.0.0.1`.

```c++
#include <yaskawa_ethernet/udp/client.hpp>

#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
namespace yaskawa = dr::yaskawa;

using Client = dr::yaskawa::udp::Client;
using Error  = dr::yaskawa::Error;

template<typename T>
using Result = dr::yaskawa::Result<T>;


int main() {
	asio::io_context io_context;
	int exit_status = 1;

	// Create the UDP client.
	Client client(io_context);

	// Connect to 10.0.0.1.
	// Note that this is UDP, so it merely sets the default peer address for the socket.
	// A succesfull "connect" is no guarantee that the communication is working.
	client.connect("10.0.0.1", 10040, 300ms, [&] (Error error) {
		if (error) {
			std::cerr << "Failed to connect: " << error.format() << "\n";
			return;
		}

		// Send a command to read D-variable 1.
		client.sendCommand(yaskawa::ReadInt32Var{1}, 100ms, [&] (yaskawa::Result<std::int32_t> value) {
			if (!value) {
				std::cerr << "Failed to read D001: " << value.error().format() << "\n";
				return;
			}

			std::cerr << "D001: " << *value << "\n";
			exit_status = 0;
		});
	});

	io_context.run();
	return exit_status;
}
```
