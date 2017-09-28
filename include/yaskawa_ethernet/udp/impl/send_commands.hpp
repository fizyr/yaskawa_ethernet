#pragma once
#include "../client.hpp"

#include <chrono>
#include <type_traits>

namespace dr {
namespace yaskawa {
namespace udp {

namespace impl {

template<typename T, typename From, typename To>
struct map_type_impl { using type = T; };

template<typename From, typename To>
struct map_type_impl<From, From, To> { using type = To; };

/// Map type From to type To, return other types unmodified.
template<typename T, typename From, typename To>
using map_type = typename map_type_impl<T, From, To>::type;

/// Map void to type To (defaults to std::nullptr_t), return other types unmodified.
template<typename T, typename To = std::nullptr_t>
using map_void = map_type<T, void, To>;

template<std::size_t Total, typename MultiCommandSession, typename Head, typename... Tail>
void startCommands(Client & client, MultiCommandSession & session, std::chrono::steady_clock::time_point deadline, Head && head, Tail && ...tail) {
	constexpr std::size_t I = Total - sizeof...(Tail) - 1;
	session.template setStop<I>(client.sendCommand(std::forward<Head>(head), deadline, session.template callback<I, Head>()));
	startCommands<Total, MultiCommandSession, Tail...>(client, session, deadline, std::forward<Tail>(tail)...);
}

template<std::size_t Total, typename MultiCommandSession>
void startCommands(Client &, MultiCommandSession &, std::chrono::steady_clock::time_point) {}

template<typename MultiCommandSession, typename... Commands>
void startCommands(Client & client, MultiCommandSession & session, std::chrono::steady_clock::time_point deadline, Commands && ... commands) {
	startCommands<sizeof...(Commands), MultiCommandSession, Commands...>(client, session, deadline, std::forward<Commands>(commands)...);

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

template<typename... Commands>
class MultiCommandSessionImpl : public std::enable_shared_from_this<MultiCommandSessionImpl<Commands...>> {
public:
	using ResponseTuple = std::tuple<map_void<typename Commands::Response>...>;
	using Callback       = std::function<void (ErrorOr<ResponseTuple>)>;

private:
	constexpr static int Count = sizeof...(Commands);

	std::atomic<int> finished_commands_{0};
	std::atomic_flag done_ = ATOMIC_FLAG_INIT;
	Callback callback_;

	ResponseTuple result_;
	std::array<std::function<void ()>, sizeof...(Commands)> stop_callbacks_;

public:
	MultiCommandSessionImpl(Callback callback) :
		callback_{std::move(callback)} {}

	std::function<void()> start(Client & client, std::chrono::steady_clock::time_point deadline, Commands ...commands) {
		startCommands(client, *this, deadline, std::move(commands)...);
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
	std::shared_ptr<MultiCommandSessionImpl> self() {
		return this->shared_from_this();
	}
};

template<typename... Commands>
using MultiCommandSession = MultiCommandSessionImpl<std::decay_t<Commands>...>;

template<typename... Commands>
std::function<void()> sendMultipleCommands(
	Client & client,
	std::chrono::steady_clock::time_point deadline,
	typename MultiCommandSession<Commands...>::Callback callback,
	Commands && ...commands
) {
	using Session = MultiCommandSession<Commands...>;
	auto session = std::make_shared<Session>(std::move(callback));
	return session->start(client, deadline, std::forward<Commands>(commands)...);
}

}}}}
