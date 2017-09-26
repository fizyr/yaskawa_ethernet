#pragma once
#include "protocol.hpp"
#include "../commands.hpp"
#include "../error.hpp"

#include <dr_error/error_or.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

namespace dr {
namespace yaskawa {
namespace tcp {

class Client {
public:
	using Socket   = asio::ip::tcp::socket;
	using Callback = std::function<void (std::error_code const & error)>;

	template<typename T>
	using ResultCallback = std::function<void (ErrorOr<T> const & result)>;

private:
	Socket socket_;
	asio::streambuf read_buffer_;

public:
	Client(asio::io_service & ios);

	/// Open a connection.
	void connect(
		std::string const & host,          ///< Hostname or IP address to connect to.
		std::string const & port,          ///< Port number or service name to connect to.
		std::chrono::milliseconds timeout, ///< Timeout for the connection attempt in milliseconds.
		Callback const & callback          ///< Callback to call when the connection attempt finished.
	);

	/// Open a connection.
	void connect(
		std::string const & host,          ///< Hostname or IP address to connect to.
		std::uint16_t port,                ///< Port number to connect to.
		std::chrono::milliseconds timeout, ///< Timeout for the connection attempt in milliseconds.
		Callback const & callback          ///< Callback to call when the connection attempt finished.
	);

	/// Close the connection.
	void close();

	/// Get the IO service.
	asio::io_service & ios() { return socket_.get_io_service(); }

	/// Get the socket used by the client.
	Socket       & socket()       { return socket_; }
	Socket const & socket() const { return socket_; }

	/// Start the connection.
	void start(int keep_alive, ResultCallback<std::string> function);

	void enableServo(bool enable, ResultCallback<void> callback);
	void startJob(std::string const & job, ResultCallback<void> callback);

	void readPulsePosition(ResultCallback<PulsePosition> callback);
	void readCartesianPosition(CoordinateSystem system, ResultCallback<CartesianPosition> callback);

	void readIo(unsigned int start, unsigned int count, ResultCallback<std::vector<std::uint8_t>> callback);
	void writeIo(unsigned int start, std::vector<std::uint8_t> const & data, ResultCallback<void> callback);

	void readByteVariable     (int index, ResultCallback<std::uint8_t> callback);
	void readIntVariable      (int index, ResultCallback<std::int16_t> callback);
	void readDoubleIntVariable(int index, ResultCallback<std::int32_t> callback);
	void readRealVariable     (int index, ResultCallback<float>        callback);
	void readPositionVariable (int index, ResultCallback<Position>     callback);

	void writeByteVariable     (int index, std::uint8_t value, ResultCallback<void> callback);
	void writeIntVariable      (int index, std::int16_t value, ResultCallback<void> callback);
	void writeDoubleIntVariable(int index, std::int32_t value, ResultCallback<void> callback);
	void writeRealVariable     (int index, float        value, ResultCallback<void> callback);
	void writePositionVariable (int index, Position     value, ResultCallback<void> callback);
};

}}}
