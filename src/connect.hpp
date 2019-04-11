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
#include <memory>
#include <atomic>

#include <asio/steady_timer.hpp>
#include <asio/connect.hpp>

#include <functional>

namespace dr {
namespace yaskawa {

/// Class representing a connection attempt.
template<typename Socket, typename Callback, typename Resolver = typename Socket::protocol_type::resolver>
class ConnectionAttempt : public std::enable_shared_from_this<ConnectionAttempt<Socket, Callback, Resolver>> {
	using Ptr      = std::shared_ptr<ConnectionAttempt>;
	using Iterator = typename Resolver::iterator;
	using Query    = typename Resolver::query;

	/// The socket to connect with.
	Socket * socket;

	/// The callback to invoke on success, failure or timeout.
	Callback callback;

	/// Resolver to perform the query with.
	Resolver resolver;

	/// Timer to keep track of the timeout with.
	asio::steady_timer timer;

	/// Flag to remember if the callback has been invoked already.
	std::atomic<bool> finished{false};

public:
	/// Create a connection attempt.
	explicit ConnectionAttempt(Socket & socket, Callback callback) :
		socket(&socket),
		callback(std::move(callback)),
		resolver(socket.get_io_service()),
		timer(socket.get_io_service()) {}

	/// Start the connection attempt.
	void start(
		Query query,                      ///< The resolver query.
		std::chrono::milliseconds timeout ///< Time timout for the full connection attempt in milliseconds, or 0 for no timeout.
	) {
		auto callback = std::bind(&ConnectionAttempt::onResolve, this, self(), std::placeholders::_1, std::placeholders::_2);
		resolver.async_resolve(query, callback);
		if (timeout.count()) {
			timer.expires_from_now(timeout);
			timer.async_wait(std::bind(&ConnectionAttempt::onConnectTimeout, this, self(), std::placeholders::_1));
		}
	}

protected:
	/// Get a shared pointer to this connection attempt.
	Ptr self() { return this->shared_from_this(); }

	/// Called when the resolver has a result.
	void onResolve(Ptr, std::error_code const & error, Iterator iterator) {
		if (error) {
			if (finished.exchange(true)) return;
			callback(make_error_code(std::errc(error.value())));
		}

		if (finished.load()) return;

		auto connect_callback = std::bind(&ConnectionAttempt::onConnect, this, self(), std::placeholders::_1, std::placeholders::_2);
		asio::async_connect(*socket, iterator, connect_callback);
	}

	/// Called when a connection attempt finished.
	void onConnect(Ptr, std::error_code const & error, Iterator iterator) {
		(void) iterator;

		if (finished.exchange(true)) {
			std::error_code discard_error;
			socket->close(discard_error);
			return;
		}

		finished = true;
		timer.cancel();
		callback(make_error_code(std::errc(error.value())));
	}

	/// Called when a connection attempt times out.
	void onConnectTimeout(Ptr, std::error_code const & error) {
		if (finished.exchange(true)) return;
		resolver.cancel();
		socket->cancel();

		if (error) return callback(make_error_code(std::errc(error.value())));
		callback(make_error_code(std::errc::timed_out));
	}
};

/// Perform an asynchronous connection attempt.
template<typename Socket, typename Callback, typename Resolver = typename Socket::protocol_type::resolver>
void asyncResolveConnect(
	typename Resolver::query query,    ///< The resolver query.
	std::chrono::milliseconds timeout, ///< The timeout in milliseconds, or 0 for no timeout.
	Socket & socket,                   ///< The socket to connect with.
	Callback callback                  ///< The callback to invoke on success, failure or timeout.
) {
	auto connection_attempt = std::make_shared<ConnectionAttempt<Socket, Callback, Resolver>>(socket, callback);
	connection_attempt->start(query, timeout);
}

}}
