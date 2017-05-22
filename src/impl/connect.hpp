#pragma once
#include <memory>
#include <atomic>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/connect.hpp>

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
	boost::asio::steady_timer timer;

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
	void onResolve(Ptr, boost::system::error_code const & error, Iterator iterator) {
		if (error) {
			if (finished.exchange(true)) return;
			callback(make_error_code(std::errc(error.value())));
		}

		if (finished.load()) return;

		auto connect_callback = std::bind(&ConnectionAttempt::onConnect, this, self(), std::placeholders::_1, std::placeholders::_2);
		boost::asio::async_connect(*socket, iterator, connect_callback);
	}

	/// Called when a connection attempt finished.
	void onConnect(Ptr, boost::system::error_code const & error, Iterator iterator) {
		(void) iterator;

		if (finished.exchange(true)) {
			boost::system::error_code discard_error;
			socket->close(discard_error);
			return;
		}

		finished = true;
		timer.cancel();
		callback(make_error_code(std::errc(error.value())));
	}

	/// Called when a connection attempt times out.
	void onConnectTimeout(Ptr, boost::system::error_code const & error) {
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
