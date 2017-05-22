#pragma once
#include "../error.hpp"
#include "../types.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/streambuf.hpp>

#include <dr_error/error_or.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

namespace dr {
namespace yaskawa {
namespace udp {

class Client {
public:
	using Socket   = boost::asio::ip::udp::socket;
	using Callback = std::function<void (std::error_code const & error)>;

	template<typename T>
	using ResultCallback = std::function<void (ErrorOr<T> const & result)>;

private:
	Socket socket_;
	std::uint8_t request_id_ = 1;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;
	std::ostream write_stream_{&write_buffer_};

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

	/// Get the IO service used by the client.
	boost::asio::io_service & ios() { return socket_.get_io_service(); }

	/// Get the socket used by the client.
	Socket        & socket()       { return socket_; }
	Socket const  & socket() const { return socket_; }

	void readByte(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::uint8_t>)> callback);
	void writeByte(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readInt16(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::int16_t>)> callback);
	void writeInt16(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readInt32(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<std::int32_t>)> callback);
	void writeInt32(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readFloat32(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<float>)> callback);
	void writeFloat32(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);

	void readRobotPosition(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback);
	void writeRobotPosition(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback);
};

}}}
