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
	DeadlineSession(asio::executor executor, Args && ...args) :
		timer_{executor},
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
