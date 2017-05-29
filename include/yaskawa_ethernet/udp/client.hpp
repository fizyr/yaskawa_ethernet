#pragma once
#include "../error.hpp"
#include "../types.hpp"
#include "../string_view.hpp"
#include "message.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/streambuf.hpp>

#include <dr_error/error_or.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <map>

namespace dr {
namespace yaskawa {
namespace udp {

class Client {
public:
	using Socket   = boost::asio::ip::udp::socket;
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
	Client(boost::asio::io_service & ios);

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
	boost::asio::io_service & ios() { return socket_.get_io_service(); }

	/// Get the socket used by the client.
	Socket        & socket()       { return socket_; }
	Socket const  & socket() const { return socket_; }

	/// Register a handler for a request id.
	HandlerToken registerHandler(std::uint8_t request_id, std::function<void(ResponseHeader const &, string_view)> handler);

	/// Remove a handler for a request id.
	void removeHandler(HandlerToken);

	void readByte(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::uint8_t>)> callback);
	void readBytes(int index, int count, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::vector<std::uint8_t>> const &)> callback);
	void writeByte(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
	void writeBytes(int index, std::vector<std::uint8_t> const & value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readInt16(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::int16_t>)> callback);
	void readInt16s(int index, int count, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::vector<std::int16_t>> const &)> callback);
	void writeInt16(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
	void writeInt16s(int index, std::vector<std::int16_t> const & value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readInt32(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::int32_t>)> callback);
	void readInt32s(int index, int count, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::vector<std::int32_t>> const &)> callback);
	void writeInt32(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
	void writeInt32s(int index, std::vector<std::int32_t> const & value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readFloat32(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<float>)> callback);
	void readFloat32s(int index, int count, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::vector<float>> const &)> callback);
	void writeFloat32(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
	void writeFloat32s(int index, std::vector<float> const & value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readRobotPosition(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback);
	void readRobotPositions(int index, int count, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::vector<Position>> const &)> callback);
	void writeRobotPosition(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
	void writeRobotPositions(int index, std::vector<Position> const & value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

private:
	/// Called when a connection attempt finishes.
	void onConnect(DetailedError, Callback callback);

	/// Start an asynchronous receive.
	void receive();

	/// Process incoming messages.
	void onReceive(boost::system::error_code error, std::size_t message_size);
};

}}}
