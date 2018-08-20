#pragma once
#include <estd/result.hpp>

#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <asio/error.hpp>

#include <chrono>
#include <utility>
#include <system_error>

namespace dr {
namespace yaskawa {
namespace udp {
namespace impl {

template<typename Session>
class DeadlineSession {
public:
	/// Type passed to the callback.
	using result_type = typename Session::result_type;

private:
	/// Timer to interrupt the work session.
	asio::steady_timer timer_;

	/// The session doing the real work.
	Session work_;

public:
	template<typename ...Args>
	DeadlineSession(asio::io_service & io_service, Args && ...args) :
		timer_{io_service},
		work_(std::forward<Args>(args)...) {}

	template<typename ...Args>
	void start(std::chrono::steady_clock::time_point deadline, Args && ...args) {
		work_.start(std::forward<Args>(args)...);

		timer_.expires_at(deadline);
		timer_.async_wait([this] (std::error_code error) {
			if (error == asio::error::operation_aborted) return;
			if (error) work_.resolve(estd::error{error, "waiting for timeout"});
			work_.resolve(estd::error{asio::error::timed_out});
		});
	}

	template<typename ...Args>
	void start(std::chrono::steady_clock::duration timeout, Args && ...args) {
		start(std::chrono::steady_clock::now() + timeout, std::forward<Args>(args)...);
	}

	void resolve(result_type result) {
		work_.resolve(std::move(result));
	}

	void cancelTimeout() {
		timer_.cancel();
	}
};

}}}}
