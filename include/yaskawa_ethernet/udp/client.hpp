#pragma once
#include "../error.hpp"
#include "../commands.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/streambuf.hpp>

#include <dr_error/error_or.hpp>

#include <functional>
#include <cstdint>
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

	void readByteVariable(ReadInt8Variable::Request request, unsigned int timeout, ResultCallback<ReadInt8Variable::Response> const & callback);
	void writeByteVariable(WriteInt8Variable::Request request, unsigned int timeout, ResultCallback<WriteInt8Variable::Response> const & callback);
};

}}}
