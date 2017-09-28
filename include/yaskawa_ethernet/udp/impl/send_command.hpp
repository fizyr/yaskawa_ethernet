#pragma once
#include "../client.hpp"
#include "../protocol.hpp"
#include "../../error.hpp"
#include "../../string_view.hpp"

#include <asio/steady_timer.hpp>
#include <asio/buffer.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <system_error>
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
template<typename Command, typename Callback>
class CommandSession : public std::enable_shared_from_this<CommandSession<Command, Callback>> {
	using Response = typename Command::Response;

	Client * client_;
	std::uint8_t request_id_;
	Command command_;
	Callback callback_;

	Client::HandlerToken handler_;
	asio::steady_timer timer_;
	std::vector<std::uint8_t> write_buffer_;

	std::atomic<bool> done_{false};

public:
	/// Construct a command session.
	CommandSession(Client & client, std::uint8_t request_id, Command command, Callback callback) :
		client_{&client},
		request_id_{request_id},
		command_{std::move(command)},
		callback_{std::move(callback)},
		timer_{client.ios()}
	{}

	auto start(std::chrono::steady_clock::time_point deadline) {
		// Encode the command.
		encode(write_buffer_, request_id_, command_);

		// Register the response handler.
		handler_ = client_->registerHandler(request_id_, [this, self = self()] (ResponseHeader const & header, string_view data) {
			stopSession(decode(header, data, command_));
		});

		// Write the command.
		client_->socket().async_send(asio::buffer(write_buffer_.data(), write_buffer_.size()), [this, self = self()] (std::error_code error, std::size_t) {
			if (error) stopSession(DetailedError{error, "writing command for request " + std::to_string(request_id_)});
		});

		// Start the timout.
		resetTimeout(deadline);

		// Return a lambda to stop the session.
		return [this, self = self()] () {
			cancel();
		};
	}

	// Stop the session as soon as possible.
	void cancel() {
		stopSession(DetailedError{asio::error::operation_aborted});
	}

protected:
	/// Get a shared pointer to this session.
	std::shared_ptr<CommandSession> self() { return this->shared_from_this(); }

	/// Called when the request times out.
	void resetTimeout(std::chrono::steady_clock::time_point deadline) {
		timer_.expires_at(deadline);
		timer_.async_wait([this, self = self()] (std::error_code error) {
			if (error == asio::error::operation_aborted) return;
			if (error) return stopSession(DetailedError(error, "waiting for reply to request " + std::to_string(request_id_)));
			stopSession(DetailedError(std::errc::timed_out, "waiting for reply to request " + std::to_string(request_id_)));
		});
	}

	void stopSession(ErrorOr<Response> result) {
		if (done_.exchange(true)) return;
		timer_.cancel();
		client_->removeHandler(handler_);
		return callback_(result);
	}
};

/// Start a command session.
/**
 * \returns a functor to stop the session.
 */
template<typename Command, typename Callback>
std::function<void()> sendCommand(Client & client, std::uint8_t request_id, Command command, std::chrono::steady_clock::time_point deadline, Callback callback) {
	using Session = impl::CommandSession<std::decay_t<Command>, std::decay_t<Callback>>;
	auto session = std::make_shared<Session>(client, request_id, std::move(command), std::forward<Callback>(callback));
	return session->start(deadline);
}

}}}}
