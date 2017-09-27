#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../types.hpp"
#include "../string_view.hpp"
#include "message.hpp"

#include <asio/io_service.hpp>
#include <asio/ip/udp.hpp>
#include <asio/streambuf.hpp>

#include <dr_error/error_or.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <map>

namespace dr {
namespace yaskawa {
namespace udp {

class Client;

namespace impl {
	template<typename Command, typename Callback>
	void sendCommand(Client & client, std::uint8_t request_id, Command command, std::chrono::milliseconds timeout, Callback callback);
}

class Client {
public:
	using Socket   = asio::ip::udp::socket;
	using Callback = std::function<void (DetailedError error)>;

	struct OpenRequest {
		std::chrono::steady_clock::time_point start_time;
		std::function<void (ResponseHeader const & header, string_view data)> on_reply;
	};

	using HandlerToken = std::map<std::uint8_t, OpenRequest>::iterator;

	Callback on_error;

private:
	Socket socket_;
	std::uint8_t request_id_ = 1;
	std::unique_ptr<std::array<std::uint8_t, 512>> read_buffer_;

	std::map<std::uint8_t, OpenRequest> requests_;

public:
	Client(asio::io_service & ios);

	/// Open a connection.
	void connect(
		std::string const & host,          ///< Hostname or IP address to connect to.
		std::string const & port,          ///< Port number or service name to connect to.
		std::chrono::milliseconds timeout, ///< Timeout for the connection attempt in milliseconds.
		Callback callback                  ///< Callback to call when the connection attempt finished.
	);

	/// Open a connection.
	void connect(
		std::string const & host,          ///< Hostname or IP address to connect to.
		std::uint16_t port,                ///< Port number to connect to.
		std::chrono::milliseconds timeout, ///< Timeout for the connection attempt in milliseconds.
		Callback callback                  ///< Callback to call when the connection attempt finished.
	);

	/// Close the connection.
	void close();

	/// Get the IO service used by the client.
	asio::io_service & ios() { return socket_.get_io_service(); }

	/// Get the socket used by the client.
	Socket        & socket()       { return socket_; }
	Socket const  & socket() const { return socket_; }

	/// Register a handler for a request id.
	HandlerToken registerHandler(std::uint8_t request_id, std::function<void(ResponseHeader const &, string_view)> handler);

	/// Remove a handler for a request id.
	void removeHandler(HandlerToken);

	template<typename T, typename Callback>
	void sendCommand(T command, std::chrono::milliseconds timeout, Callback callback) {
		impl::sendCommand(*this, request_id_++, std::move(command), timeout, std::move(callback));
	}

	void readFileList(
		std::string type,
		std::chrono::milliseconds timeout,
		std::function<void(ErrorOr<std::vector<std::string>>)> callback,
		std::function<void(std::size_t bytes_received)> on_progress
	);

	void readFile(
		std::string name,
		std::chrono::milliseconds timeout,
		std::function<void(ErrorOr<std::string>)> on_done,
		std::function<void(std::size_t bytes_received)> on_progress
	);
	void writeFile(
		std::string name,
		std::string data,
		std::chrono::milliseconds timeout,
		std::function<void(ErrorOr<void>)> on_done,
		std::function<void(std::size_t bytes_sent, std::size_t bytes_total)> on_progress
	);

	void deleteFile(
		std::string name,
		std::chrono::milliseconds timeout,
		std::function<void(ErrorOr<void>)> callback
	);

private:
	/// Called when a connection attempt finishes.
	void onConnect(DetailedError, Callback callback);

	/// Start an asynchronous receive.
	void receive();

	/// Process incoming messages.
	void onReceive(std::error_code error, std::size_t message_size);
};

}}}

#include "impl/send_command.hpp"
