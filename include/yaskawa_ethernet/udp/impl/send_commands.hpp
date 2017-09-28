#pragma once
#include "../client.hpp"

#include <chrono>
#include <type_traits>

namespace dr {
namespace yaskawa {
namespace udp {

namespace impl {

/// Helper to map a type From to type To (for the case where T != From).
template<typename T, typename From, typename To>
struct map_type_impl { using type = T; };

/// Helper to map a type From to type To (for the case where T == From).
template<typename From, typename To>
struct map_type_impl<From, From, To> { using type = To; };

/// Map type From to type To, return other types unmodified.
template<typename T, typename From, typename To>
using map_type = typename map_type_impl<T, From, To>::type;

/// Map void to type To (defaults to std::nullptr_t), return other types unmodified.
template<typename T, typename To = std::nullptr_t>
using map_void = map_type<T, void, To>;

/// Helper function to unpack a Command tuple and get the type of a Response tuple.
template<typename... Commands>
auto response_tuple_impl(std::tuple<Commands...> t) -> std::tuple<map_void<typename std::decay_t<Commands>::Response>...>;

/// Response tuple to go with a Command tuple.
/**
 * Each type T of the input tuple the tuple is replaced with T::Response,
 * or std::nullptr_t if T::Response is void.
 */
template<typename Tuple>
using ResponseTuple = decltype(response_tuple_impl(std::declval<Tuple>()));

/// Forward declaration of MultiCommandSession.
template<typename Commands> class MultiCommandSession;

/// Start all given commands and return an array of their stop functors (helper).
template<typename Commands, std::size_t... Is>
std::array<std::function<void()>, sizeof...(Is)> startCommandsImpl(Client & client, MultiCommandSession<Commands> & session, std::chrono::steady_clock::time_point deadline, Commands && commands, std::index_sequence<Is...>) {
	return {{
		client.sendCommand(std::move(std::get<Is>(commands)), deadline, session.template callback<Is, std::tuple_element_t<Is, Commands>>())...,
	}};
}

/// Start all given commands and return an array of their stop functors.
template<typename Commands>
std::array<std::function<void()>, std::tuple_size<Commands>::value> startCommands(
	Client & client,
	MultiCommandSession<Commands> & session,
	std::chrono::steady_clock::time_point deadline,
	Commands && commands
) {
	constexpr std::size_t N = std::tuple_size<Commands>::value;
	return startCommandsImpl(client, session, deadline, std::forward<Commands>(commands), std::make_index_sequence<N>());
}

/// Store something in a tuple, unless it is void, then do nothing.
template<std::size_t Index, typename Tuple, typename T>
std::enable_if_t<std::is_same<decltype(*std::declval<T>()), void>() == false>
move_deref_in_tuple(Tuple & tuple, T && value) {
	std::get<Index>(tuple) = std::move(*value);
}

/// Store something in a tuple, unless it is void, then do nothing (void version).
template<std::size_t Index, typename Tuple, typename T>
std::enable_if_t<std::is_same<decltype(*std::declval<T>()), void>() == true>
move_deref_in_tuple(Tuple &, T &&) {}

template<typename Commands>
class MultiCommandSession : public std::enable_shared_from_this<MultiCommandSession<Commands>> {
public:
	using Callback = std::function<void (ErrorOr<ResponseTuple<Commands>>)>;

private:
	constexpr static int Count = std::tuple_size<Commands>::value;

	std::atomic<int> finished_commands_{0};
	std::atomic_flag done_ = ATOMIC_FLAG_INIT;
	Callback callback_;

	ResponseTuple<Commands> result_;
	std::array<std::function<void ()>, Count> stop_callbacks_;

public:
	MultiCommandSession(Callback callback) :
		callback_{std::move(callback)} {}

	std::function<void()> start(Client & client, std::chrono::steady_clock::time_point deadline, Commands && commands) {
		stop_callbacks_ = startCommands(client, *this, deadline, std::move(commands));
		return [this, self = self()] () {
			resolve(DetailedError{asio::error::operation_aborted});
		};
	}

	template<std::size_t I> void setStop(std::function<void ()> && stop_callback) {
		static_assert(I < Count, "stop callback index exceeds valid range");
		stop_callbacks_[I] = std::move(stop_callback);
	}

	template<std::size_t I, typename Command>
	auto callback() {
		static_assert(I < Count, "command callback index exceeds valid range");
		using Response = typename Command::Response;

		return [this, self = self()] (ErrorOr<Response> result) {
			if (!result) return resolve(result.error_unchecked());
			move_deref_in_tuple<I>(result_, std::move(result));
			if (++finished_commands_ == Count) resolve(DetailedError{});
		};
	}

	void resolve(DetailedError error) {
		if (done_.test_and_set()) return;
		if (error) {
			for (auto const & f : stop_callbacks_) f();
			std::move(callback_)(std::move(error));
		} else {
			std::move(callback_)(std::move(result_));
		}
		for (auto & f : stop_callbacks_) f = nullptr;
	}

private:
	std::shared_ptr<MultiCommandSession> self() {
		return this->shared_from_this();
	}
};

template<typename Commands>
std::function<void()> sendMultipleCommands(
	Client & client,
	std::chrono::steady_clock::time_point deadline,
	typename MultiCommandSession<Commands>::Callback callback,
	Commands && commands
) {
	using Session = MultiCommandSession<Commands>;
	auto session = std::make_shared<Session>(std::move(callback));
	return session->start(client, deadline, std::forward<Commands>(commands));
}

}}}}
