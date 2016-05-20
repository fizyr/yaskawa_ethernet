#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "send_command.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <functional>
#include <cstdint>
#include <string>

namespace dr {
namespace yaskawa {
namespace tcp {

class Client {
public:
	using Socket   = boost::asio::ip::tcp::socket;
	using Callback = std::function<void (boost::system::error_code const & error)>;

	template<typename T>
	using ResultCallback = std::function<void (ErrorOr<T> const & result)>;

private:
	Socket socket_;
	boost::asio::streambuf read_buffer_;

public:
	Client(boost::asio::io_service & ios);

	/// Open a connection.
	void connect(
		std::string const & host,   ///< Hostname or IP address to connect to.
		std::string const & port,   ///< Port number or service name to connect to.
		unsigned int timeout,       ///< Timeout for the connection attempt in milliseconds.
		Callback const & callback   ///< Callback to call when the connection attempt finished.
	);

	/// Open a connection.
	void connect(
		std::string const & host,   ///< Hostname or IP address to connect to.
		std::uint16_t port,         ///< Port number to connect to.
		unsigned int timeout,       ///< Timeout for the connection attempt in milliseconds.
		Callback const & callback   ///< Callback to call when the connection attempt finished.
	);

	/// Close the connection.
	void close();

	boost::asio::io_service & ios() { return socket_.get_io_service(); }

	/// Send a command over the server.
	template<typename Command, typename Callback>
	void sendCommand(typename Command::Request const & request, Callback && callback) {
		tcp::sendCommand<Command>(request, socket_, read_buffer_, std::forward<Callback>(callback));
	}

	/// Start the connection.
	template<typename Callback>
	void start(int keep_alive, Callback && callback) {
		sendCommand<StartCommand>(StartCommand::Request{keep_alive}, std::forward<Callback>(callback));
	}
};

}}}
