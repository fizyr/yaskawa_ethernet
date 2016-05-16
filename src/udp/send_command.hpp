#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "udp/protocol.hpp"

#include <boost/asio/streambuf.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/container/vector.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {
namespace impl {

/// Class representing a command session.
/**
 * The session consists of the following actions:
 * - Write command and data.
 * - Read command response.
 * - Read response data.
 */
template<typename Command, typename Socket, typename Callback>
class CommandSession : public std::enable_shared_from_this<CommandSession<Command, Socket, Callback>> {
	using Request  = typename Command::Request;
	using Response = typename Command::Response;
	using Ptr = std::shared_ptr<CommandSession>;

	Socket * socket;
	boost::asio::steady_timer timer;
	std::vector<std::uint8_t> write_buffer;
	boost::container::vector<std::uint8_t> read_buffer;

	std::atomic_bool done{false};
	Callback callback;

public:
	/// Construct a command session.
	CommandSession(Socket & socket, Callback && callback) :
		socket(&socket),
		timer(socket.get_io_service()),
		callback(std::move(callback)) {}

	/// Construct a command session.
	CommandSession(Socket & socket, Callback const & callback) :
		socket(&socket),
		timer(socket.get_io_service()),
		callback(callback) {}

	/// Start the session by writing the command and starting the timeout..
	void start(Request const & request, unsigned int timeout, std::uint8_t request_id) {
		write_buffer = encode(request, request_id);
		auto callback = std::bind(&CommandSession::onWriteCommand, this, self(), std::placeholders::_1, std::placeholders::_2);
		socket->async_send(boost::asio::buffer(write_buffer.data(), write_buffer.size()), callback);
		timer.expires_from_now(std::chrono::milliseconds(timeout));
		timer.async_wait(std::bind(&CommandSession::onTimeout, this, self(), std::placeholders::_1));
	}

protected:
	/// Get a shared pointer to this session.
	Ptr self() { return this->shared_from_this(); }

	/// Called when the command and data have been written.
	void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done.load()) return;
		write_buffer.clear();
		if (error) return callback(error);

		read_buffer.resize(512, boost::container::default_init);
		auto callback = std::bind(&CommandSession::onReadResponse, this, self(), std::placeholders::_1, std::placeholders::_2);
		socket->async_receive(boost::asio::buffer(read_buffer.data(), read_buffer.size()), callback);
	}

	/// Called when the command response has been read.
	void onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
		if (done.exchange(true)) return;
		timer.cancel();
		read_buffer.resize(bytes_transferred);
		if (error) return callback(error);
		ErrorOr<Response> response = decode<Response>(string_view{reinterpret_cast<char *>(read_buffer.data()), read_buffer.size()});
		callback(response);
	}

	/// Called when the request times out.
	void onTimeout(Ptr, boost::system::error_code const & error) {
		if (done.exchange(true)) return;
		socket->cancel();
		if (error) return callback(error);
		callback(make_error_code(boost::asio::error::timed_out));
	}
};

template<typename Command, typename Socket, typename Callback>
void sendCommand(typename Command::Request const & request, unsigned int timeout, std::uint8_t request_id, Socket & socket, Callback && callback) {
	auto session = std::make_shared<impl::CommandSession<Command, Socket, typename std::decay<Callback>::type>>(socket, std::forward<Callback>(callback));
	session->start(request, timeout, request_id);
}

}}}}
