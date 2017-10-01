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

namespace detail {
	template<typename Command>
	struct response_tuple_element {
		using type = map_type_t<typename Command::Response, void, std::nullptr_t>;
	};

	template<typename Command>
	struct command_session_tuple_element {
		using type = std::optional<CommandSession<Command>>;
	};
}

template<typename Commands>
class MultiCommandSession {
	constexpr static int Count = std::tuple_size<Commands>::value;

	using ResponseTuple        = map_tuple_t<Commands, detail::response_tuple_element>;
	using CommandSessionsTuple = map_tuple_t<Commands, detail::command_session_tuple_element>;

public:
	using result_type = ErrorOr<ResponseTuple>;

private:
	/// Sub-sessions.
	CommandSessionsTuple sessions_;

	/// Result storage.
	ResponseTuple result_;

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

		return [this] (ErrorOr<Response> result) {
			if (!result) return resolve(result.error_unchecked());
			if constexpr (std::is_same<Response, void>() == false) {
				std::get<I>(result_) = std::move(*result);
			}
			if (++finished_commands_ == Count) resolve(DetailedError{});
		};
	}

	void resolve(DetailedError error) {
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
	void stop_sessions_(DetailedError const & error) {
		if constexpr(I < Count) {
			std::get<I>(sessions_)->resolve(error);
			stop_sessions_<I + 1>(error);
		}
	}
};

template<typename Commands>
auto sendMultipleCommands(
	Client & client,
	std::chrono::steady_clock::time_point deadline,
	std::function<void(typename MultiCommandSession<Commands>::result_type)> callback,
	Commands && commands
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
