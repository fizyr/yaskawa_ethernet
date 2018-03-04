#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../types.hpp"
#include "message.hpp"

#include <asio/io_service.hpp>
#include <asio/ip/udp.hpp>
#include <asio/streambuf.hpp>

#include <dr_error/error_or.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace dr {
namespace yaskawa {
namespace udp {

class Client {
public:
	using Socket   = asio::ip::udp::socket;
	using ErrorCallback = std::function<void (DetailedError error)>;

	struct OpenRequest {
		std::chrono::steady_clock::time_point start_time;
		std::function<void (ResponseHeader const & header, std::string_view data)> on_reply;
	};

	using HandlerToken = std::map<std::uint8_t, OpenRequest>::iterator;

	ErrorCallback on_error;

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
		ErrorCallback callback             ///< Callback to call when the connection attempt finished.
	);

	/// Open a connection.
	void connect(
		std::string const & host,          ///< Hostname or IP address to connect to.
		std::uint16_t port,                ///< Port number to connect to.
		std::chrono::milliseconds timeout, ///< Timeout for the connection attempt in milliseconds.
		ErrorCallback callback             ///< Callback to call when the connection attempt finished.
	);

	/// Close the connection.
	void close();

	/// Get the IO service used by the client.
	asio::io_service & ios() { return socket_.get_io_service(); }

	/// Get the socket used by the client.
	Socket        & socket()       { return socket_; }
	Socket const  & socket() const { return socket_; }

	/// Register a handler for a request id.
	HandlerToken registerHandler(std::uint8_t request_id, std::function<void(ResponseHeader const &, std::string_view)> handler);

	/// Remove a handler for a request id.
	void removeHandler(HandlerToken);

	/// Alocate a request ID.
	std::uint8_t allocateId() {
		return request_id_++;
	}

	/// Send a command.
	/**
	 * \return a functor which tries to stop the command as soon as possible when invoked.
	 */
	template<typename T, typename Callback>
	void sendCommand(T command, std::chrono::steady_clock::time_point deadline, Callback && callback);

	template<typename T, typename Callback>
	void sendCommand(T command, std::chrono::steady_clock::duration timeout, Callback && callback) {
		return sendCommand(std::forward<T>(command), std::chrono::steady_clock::now() + timeout, std::forward<Callback>(callback));
	}

	template<typename Callback, typename... Commands>
	void sendCommands(std::tuple<Commands...> commands, std::chrono::steady_clock::time_point deadline, Callback && callback);

	template<typename Callback, typename... Commands>
	void sendCommands(std::tuple<Commands...> commands, std::chrono::steady_clock::duration timeout, Callback && callback) {
		return sendCommands(std::move(commands), std::chrono::steady_clock::now() + timeout, std::forward<Callback>(callback));
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
	void onConnect(DetailedError, ErrorCallback callback);

	/// Start an asynchronous receive.
	void receive();

	/// Process incoming messages.
	void onReceive(std::error_code error, std::size_t message_size);
};

}}}

#include "impl/send_command.hpp"
#include "impl/send_multiple_commands.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

template<typename T, typename Callback>
void Client::sendCommand(T command, std::chrono::steady_clock::time_point deadline, Callback && callback) {
	impl::sendCommand(*this, std::move(command), deadline, std::forward<Callback>(callback));
}

template<typename Callback, typename... Commands>
void Client::sendCommands(std::tuple<Commands...> commands, std::chrono::steady_clock::time_point deadline, Callback && callback) {
	impl::sendMultipleCommands(*this, std::move(commands), deadline, std::forward<Callback>(callback));
}

template<typename Commands>
using MultiCommandResult = typename impl::MultiCommandSession<std::decay_t<Commands>>::result_type;
template<typename Commands>
using MultiCommandResponse = typename impl::MultiCommandSession<std::decay_t<Commands>>::response_type;

}}}
