/* Copyright 2016-2019 Fizyr B.V. - https://fizyr.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include "../../error.hpp"
#include "../client.hpp"
#include "../protocol.hpp"
#include "./deadline_session.hpp"

#include <asio/steady_timer.hpp>
#include <asio/buffer.hpp>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

namespace impl {
/// Session to send a command and parse the result.
/**
 * The session consists of the following actions:
 * - Write command and data.
 * - Read command response.
 * - Read response data.
 *
 * It does not support timeouts dreictly, but it does have a cancel() method.
 */
template<typename Command>
class CommandSession {
public:
	/// Type passed to the callback.
	using result_type = Result<typename Command::Response>;

private:
	Client * client_;
	std::uint8_t request_id_;
	Command command_;
	std::function<void(result_type)> callback_;

	Client::HandlerToken handler_;
	std::vector<std::uint8_t> write_buffer_;

	std::atomic_flag started_ = ATOMIC_FLAG_INIT;
	std::atomic_flag done_    = ATOMIC_FLAG_INIT;

public:
	/// Construct a command session.
	CommandSession(Client & client, Command command) :
		client_{&client},
		request_id_{client.allocateId()},
		command_{std::move(command)}
	{
		// Encode the command.
		encode(write_buffer_, request_id_, command_);
	}

	// Delete copy and move constructors, since we've posted callbacks with our address.
	CommandSession(CommandSession const &) = delete;
	CommandSession(CommandSession      &&) = delete;

	/// Start the session.
	/**
	 * By delaying start and taking the callback here,
	 * the callback can contain a shared pointer to keep ourselves alive.
	 */
	void start(std::function<void(result_type)> callback) {
		if (started_.test_and_set()) throw std::logic_error("CommandSession::start: session already started");
		callback_ = std::move(callback);

		// Register the response handler.
		handler_ = client_->registerHandler(request_id_, [this] (ResponseHeader const & header, std::string_view data) {
			if (header.status != 0) {
				resolve(commandFailed(header.status, header.extra_status));
			} else {
				resolve(decode(header, data, command_));
			}
		});

		// Write the command.
		client_->socket().async_send(asio::buffer(write_buffer_.data(), write_buffer_.size()), [this] (std::error_code error, std::size_t) {
			if (error) resolve(Error{error, "writing command for request " + std::to_string(request_id_)});
		});
	}

	void resolve(result_type result) {
		if (done_.test_and_set()) return;
		client_->removeHandler(handler_);
		callback_(result);
	}
};

/// Start a command session manages by a shared_ptr.
/**
 * The shared_ptr will be held internally by the session until it is ready to be destroyed,
 * so you do not need to keep your own copy alive.
 *
 * \returns a shared_ptr to the created session.
 */
template<typename Command, typename Callback>
auto sendCommand(Client & client, Command command, std::chrono::steady_clock::time_point deadline, Callback callback) {
	using Session = DeadlineSession<CommandSession<std::decay_t<Command>>>;
	auto session = std::make_shared<Session>(client.get_executor(), client, std::move(command));
	session->start(deadline, [&client, session, callback = std::move(callback)] (typename Session::result_type && result) mutable {
		session->cancelTimeout();
		std::move(callback)(std::move(result));

		// Move the shared_ptr into a posted handler which resets it.
		// That way, any queued event handlers can still completer succesfully.
		asio::post(client.get_executor(), [session = std::move(session)] () mutable {
			session.reset();
		});

		// Reset our own shared_ptr.
		// Not really needed since we've moved out of it,
		// but if the lambda is made non-mutable std::move silently doesn't move,
		// and then we have a memory leak.
		// This will make the compiler bark in that case.
		session.reset();
	});
	return session;
}

}}}}
