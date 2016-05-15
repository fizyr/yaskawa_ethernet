#pragma once
#include "error.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <functional>
#include <cstdint>
#include <string>

namespace dr {
namespace yaskawa {

class EthernetClient {
public:
	using Socket   = boost::asio::ip::tcp::socket;
	using Callback = std::function<void (boost::system::error_code const & error)>;

	template<typename T>
	using ResultCallback = std::function<void (ErrorOr<T> const & result)>;

private:
	Socket socket_;
	boost::asio::streambuf read_buffer_;
	boost::asio::streambuf write_buffer_;

public:
	EthernetClient(boost::asio::io_service & ios);

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

	void start(int keep_alive, ResultCallback<std::string> const & callback);

	void readByteVariable(int index, ResultCallback<std::uint8_t> const & callback);
	void readInt16Variable(int index, ResultCallback<std::int16_t> const & callback);
	void readInt32Variable(int index, ResultCallback<std::int32_t> const & callback);
	void readRobotPositionVariable(int index, ResultCallback<double> const & callback);
	void readStringVariable(int index, ResultCallback<std::string> const & callback);

	void writeByteVariable(int index, std::uint8_t value, ResultCallback<void> const & callback);
};

}}
