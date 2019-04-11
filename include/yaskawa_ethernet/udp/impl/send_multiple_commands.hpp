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
#include "./send_command.hpp"
#include "./deadline_session.hpp"
#include "../../type_traits.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

namespace impl {

template<typename Commands>
class MultiCommandSession {
	constexpr static int Count = std::tuple_size<Commands>::value;

	/// Map a Command to the result_type (or Empty for void results).
	template<typename Command>
	struct response_tuple_element {
		using type = map_type_t<typename Command::Response, void, Empty>;
	};

	/// Map a Command to an std::optional<CommandSession<Command>>.
	template<typename Command>
	struct command_session_tuple_element {
		using type = std::optional<CommandSession<Command>>;
	};

	using CommandSessionsTuple = map_tuple_t<Commands, command_session_tuple_element>;

public:
	using response_type  = map_tuple_t<Commands, response_tuple_element>;
	using result_type    = Result<response_type>;

private:
	/// Sub-sessions.
	CommandSessionsTuple sessions_;

	/// Result storage.
	response_type result_;

	std::atomic_flag started_ = ATOMIC_FLAG_INIT;
	std::atomic_flag done_    = ATOMIC_FLAG_INIT;

	std::atomic<int> finished_commands_{0};
	std::function<void(result_type)> callback_;

public:
	MultiCommandSession(Client & client, Commands && commands) {
		init_sessions_<0>(client, std::move(commands));
	}

public:
	void start(std::function<void(result_type)> callback) {
		if (started_.test_and_set()) throw std::logic_error("CommandSession::start: session already started");
		callback_ = std::move(callback);
		start_sessions_<0>();
	}

	template<std::size_t I>
	auto callback() {
		static_assert(I < Count, "command callback index exceeds valid range");
		using Response = typename std::tuple_element_t<I, Commands>::Response;

		return [this] (Result<Response> result) {
			if (!result) return resolve(result.error_unchecked());
			if constexpr (std::is_same<Response, void>() == false) {
				std::get<I>(result_) = std::move(*result);
			}
			if (++finished_commands_ == Count) resolve(Error{});
		};
	}

	void resolve(Error error) {
		if (done_.test_and_set()) return;
		if (error) {
			stop_sessions_<0>(asio::error::operation_aborted);
			std::move(callback_)(std::move(error));
		} else {
			std::move(callback_)(std::move(result_));
		}
	}

protected:
	/// Recursively initialize sub-sessions.
	template<std::size_t I>
	void init_sessions_(Client & client, Commands && commands) {
		if constexpr(I < Count) {
			std::get<I>(sessions_).emplace(client, std::move(std::get<I>(commands)));
			init_sessions_<I + 1>(client, std::move(commands));
		}
	}

	/// Recursively start sub-sessions.
	template<std::size_t I>
	void start_sessions_() {
		if constexpr(I < Count) {
			std::get<I>(sessions_)->start(callback<I>());
			start_sessions_<I + 1>();
		}
	}

	/// Recursively start sub-sessions.
	template<std::size_t I>
	void stop_sessions_(Error const & error) {
		if constexpr(I < Count) {
			std::get<I>(sessions_)->resolve(error);
			stop_sessions_<I + 1>(error);
		}
	}
};

template<typename Commands>
auto sendMultipleCommands(
	Client & client,
	Commands && commands,
	std::chrono::steady_clock::time_point deadline,
	std::function<void(typename MultiCommandSession<Commands>::result_type)> callback
) {
	using Session = DeadlineSession<MultiCommandSession<Commands>>;
	auto session = std::make_shared<Session>(client.ios(), client, std::move(commands));
	session->start(deadline, [&client, session, callback = std::move(callback)] (typename Session::result_type && result) mutable {
		session->cancelTimeout();
		std::move(callback)(std::move(result));

		// Move the shared_ptr into a posted handler which resets it.
		// That way, any queued event handlers can still completer succesfully.
		client.ios().post([session = std::move(session)] () mutable {
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
