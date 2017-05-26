#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "udp/client.hpp"
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
template<typename Decoder, typename Callback>
class CommandSession : public std::enable_shared_from_this<CommandSession<Decoder, Callback>> {
	using Response = decltype(std::declval<Decoder>()(std::declval<ResponseHeader const &>(), std::declval<string_view &>()));
	using Ptr      = std::shared_ptr<CommandSession>;

	Client * client;
	Client::HandlerToken handler;
	boost::asio::steady_timer timer;
	std::vector<std::uint8_t> write_buffer;

	std::atomic_bool done{false};
	Decoder decoder;
	Callback callback;

public:
	/// Construct a command session.
	CommandSession(Client & client, std::vector<uint8_t> && message, Decoder decoder, Callback callback) :
		client(&client),
		timer(client.ios()),
		write_buffer(std::move(message)),
		decoder(std::move(decoder)),
		callback(std::move(callback)) {}

	/// Start the session by writing the command and starting the timeout..
	void start(std::uint8_t request_id, std::chrono::milliseconds timeout) {
		auto callback = std::bind(&CommandSession::onWriteCommand, this, self(), std::placeholders::_1, std::placeholders::_2);
		client->socket().async_send(boost::asio::buffer(write_buffer.data(), write_buffer.size()), callback);

		auto response_callback = std::bind(&CommandSession::onResponse, this, self(), std::placeholders::_1, std::placeholders::_2);
		handler = client->registerHandler(request_id, response_callback);

		timer.expires_from_now(timeout);
		timer.async_wait(std::bind(&CommandSession::onTimeout, this, self(), std::placeholders::_1));
	}

protected:
	/// Get a shared pointer to this session.
	Ptr self() { return this->shared_from_this(); }

	/// Called when the message has been written.
	void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done.load()) return;
		if (error && done.exchange(true) == false) {
			timer.cancel();
			client->removeHandler(handler);
			callback(DetailedError(std::errc(error.value())));
		}
	}

	/// Called when the command response has been read.
	void onResponse(Ptr, ResponseHeader const & header, string_view data) {
		if (done.exchange(true)) return;
		timer.cancel();
		client->removeHandler(handler);

		Response response = decoder(header, data);
		callback(response);
	}

	/// Called when the request times out.
	void onTimeout(Ptr, boost::system::error_code const & error) {
		if (done.exchange(true)) return;
		client->removeHandler(handler);
		if (error) return callback(DetailedError(std::errc(error.value()), "waiting for reply"));
		callback(DetailedError(std::errc::timed_out, "waiting for reply"));
	}
};

template<typename Decoder, typename Callback>
void sendCommand(
	Client & client,
	std::uint8_t request_id,
	std::vector<std::uint8_t> && message,
	Decoder && decoder,
	std::chrono::milliseconds timeout,
	Callback && callback
) {
	using Session = impl::CommandSession<std::decay_t<Decoder>, std::decay_t<Callback>>;
	auto session = std::make_shared<Session>(client, std::move(message), std::forward<Decoder>(decoder),  std::forward<Callback>(callback));
	session->start(request_id, timeout);
}

}}}}
