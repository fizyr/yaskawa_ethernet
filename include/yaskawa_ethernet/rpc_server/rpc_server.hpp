#include "../udp/client.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>

namespace dr {
namespace yaskawa {

namespace detail {
	class RpcService {
	public:
		using OnExecute = std::function<void(std::function<void(DetailedError)> resolve)>;

		/// Atomic flag to remember if the service is currently busy.
		std::atomic_flag busy = ATOMIC_FLAG_INIT;
		std::string name;
		OnExecute execute;

		RpcService(std::string name, OnExecute execute) : name{std::move(name)}, execute{std::move(execute)} {}
	};
}


void disabledService(udp::Client &, std::function<void(DetailedError)> resolve) {
	resolve({std::errc::invalid_argument, "service is disabled"});
}

namespace service_status {
	constexpr std::uint8_t idle      = 0;
	constexpr std::uint8_t requested = 1;
	constexpr std::uint8_t error     = 2;
}

class RpcServer {
	/// The client to use for reading/writing command status.
	udp::Client * client_;

	/// The base register to use when reading command status.
	std::uint8_t base_register_;

	/// Vector of services.
	std::vector<std::unique_ptr<detail::RpcService>> services_;

	/// If true, we're started. If false, we should stop ASAP.
	std::atomic<bool> started_{false};

	/// A callback to invoke when an error occurs.
	std::function<void(DetailedError)> on_error_;

public:
	/// Construct a RPC server.
	RpcServer(
		udp::Client & client,                       ///< The client to use for reading/writing command status.
		std::uint8_t base_register,                 ///< The base register to use for reading/writing command status.
		std::function<void(DetailedError)> on_error ///< The callback to invoke when an error occurs.
	);

	/// Register a new service.
	/**
	 * When the service is invoked, all pre_commands are executed.
	 * If an error occurs for one of the commands, the RPC server error handler is called with the error.
	 * If all commands succeeded, the service callback is invoked as:
	 *   callback(result, resolve)
	 * where `result` is a tuple with the results of each pre_command and `resolve` is a functor taking a DetailedError
	 * that the service should invoke to notify the RPC server that the service call is finished.
	 */
	template<typename PreCommands, typename Callback>
	void addService(std::string name, PreCommands && pre_commands, std::chrono::steady_clock::duration timeout, Callback && callback) {
		services_.push_back(std::make_unique<detail::RpcService>(std::move(name)), [
			&client = *client_,
			pre_commands = std::forward<PreCommands>(pre_commands),
			timeout,
			callback = std::forward<Callback>(callback)
		] (
			std::function<void(DetailedError)> resolve
		) {
			client.sendCommands(timeout, [&client, resolve = std::move(resolve)] (udp::MultiCommandResponse<PreCommands> && result) {
				if (!result) std::move(resolve)(std::move(result.error_unchecked()));
				else callback_(*result, std::move(resolve));
			}, pre_commands);
		});
	}

	/// Start the RPC server.
	/**
	 * Does nothing if the RPC server is already started.
	 * \return False if the RPC server was already started, true otherwise.
	 */
	bool start();

	/// Stop the RPC server as soon as possible.
	/**
	 * Does nothing if the RPC server is already stopped.
	 * \return False if the RPC server was already stopped, true otherwise.
	 */
	bool stop();

protected:
	/// Read command status.
	void readCommands();

	/// Execute a service and manage the busy flag and status variable.
	bool execute(std::size_t index);
};

}}
