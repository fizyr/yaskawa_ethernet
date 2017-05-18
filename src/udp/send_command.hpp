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
template<typename Decoder, typename Socket, typename Callback>
class CommandSession : public std::enable_shared_from_this<CommandSession<Decoder, Socket, Callback>> {
	using Response = decltype(std::declval<Decoder>()(string_view{}));
	using Ptr      = std::shared_ptr<CommandSession>;

	Socket * socket;
	boost::asio::steady_timer timer;
	std::vector<std::uint8_t> write_buffer;
	boost::container::vector<std::uint8_t> read_buffer;

	std::atomic_bool done{false};
	Decoder decoder;
	Callback callback;

public:
	/// Construct a command session.
	CommandSession(std::vector<uint8_t> && message, Decoder decoder, Socket & socket, Callback callback) :
		socket(&socket),
		timer(socket.get_io_service()),
		write_buffer(std::move(message)),
		decoder(std::move(decoder)),
		callback(std::move(callback)) {}

	/// Start the session by writing the command and starting the timeout..
	
	void start(std::chrono::milliseconds timeout) {
		auto callback = std::bind(&CommandSession::onWriteCommand, this, self(), std::placeholders::_1, std::placeholders::_2);
		socket->async_send(boost::asio::buffer(write_buffer.data(), write_buffer.size()), callback);

		timer.expires_from_now(timeout);
		timer.async_wait(std::bind(&CommandSession::onTimeout, this, self(), std::placeholders::_1));
	}

protected:
	/// Get a shared pointer to this session.
	Ptr self() { return this->shared_from_this(); }

	/// Called when the command and data have been written.
	void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done.load()) return;
		write_buffer.clear();
		if (error) return callback(DetailedError(std::errc(error.value())));

		read_buffer.resize(512, boost::container::default_init);
		auto callback = std::bind(&CommandSession::onReadResponse, this, self(), std::placeholders::_1, std::placeholders::_2);
		socket->async_receive(boost::asio::buffer(read_buffer.data(), read_buffer.size()), callback);
	}

	/// Called when the command response has been read.
	void onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
		if (done.exchange(true)) return;
		timer.cancel();
		read_buffer.resize(bytes_transferred);
		if (error) return callback(DetailedError(std::errc(error.value())));
		Response response = decoder(string_view{reinterpret_cast<char *>(read_buffer.data()), read_buffer.size()});
		callback(response);
	}

	/// Called when the request times out.
	void onTimeout(Ptr, boost::system::error_code const & error) {
		if (done.exchange(true)) return;
		socket->cancel();
		if (error) return callback(DetailedError(std::errc(error.value()), "waiting for reply"));
		callback(DetailedError(std::errc::timed_out, "waiting for reply"));
	}
};

template<typename Decoder, typename Socket, typename Callback>
void sendCommand(
	std::vector<std::uint8_t> && message,
	Decoder && decoder,
	std::chrono::milliseconds timeout,
	Socket & socket,
	Callback && callback
) {
	using Session = impl::CommandSession<std::decay_t<Decoder>, Socket, std::decay_t<Callback>>;
	auto session = std::make_shared<Session>(std::move(message), std::forward<Decoder>(decoder), socket, std::forward<Callback>(callback));
	session->start(timeout);
}

}}}}
