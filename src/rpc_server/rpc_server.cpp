#include "rpc_server/rpc_server.hpp"

namespace dr {
namespace yaskawa {

using namespace std::chrono_literals;

namespace {
	DetailedError prefixError(DetailedError error, std::string prefix) {
		return {error, std::move(prefix) + std::move(error.details())};
	}
}

void disabledService(udp::Client &, std::function<void(DetailedError)> resolve) {
	resolve({std::errc::invalid_argument, "service is disabled"});
}

RpcServer::RpcServer(udp::Client & client, std::uint8_t base_register, std::chrono::steady_clock::duration delay, std::function<void(DetailedError)> on_error) :
	client_{&client},
	base_register_{base_register},
	read_commands_delay_{delay},
	read_commands_timer_{client.ios()},
	on_error_{std::move(on_error)} {}

bool RpcServer::start() {
	if (started_.exchange(true)) return false;
	readCommands();
	return true;
}

bool RpcServer::stop() {
	return started_.exchange(false);
}

void RpcServer::startReadCommandsTimer() {
	// Zero delay? Read commands immediately.
	if (read_commands_delay_.count() == 0) {
		readCommands();
		return;
	}

	// Otherwise wait for the specified delay and then execute readCommands().
	read_commands_timer_.expires_from_now(read_commands_delay_);
	read_commands_timer_.async_wait([this] (std::error_code error) {
		if (error == asio::error::operation_aborted) return;
		if (error) {
			on_error_(DetailedError{error, "waiting for read_commands_timer_"});
			startReadCommandsTimer();
			return;
		}
		readCommands();
	});
}

void RpcServer::readCommands() {
	// Read command registers.
	client_->sendCommand(ReadUint8Vars{base_register_, std::uint8_t(services_.size())}, 100ms, [this] (ErrorOr<std::vector<std::uint8_t>> const & statuses) {
		// Report error
		if (!statuses) {
			on_error_(prefixError(std::move(statuses.error_unchecked()), "reading commands status variables: "));
			if (started_) startReadCommandsTimer();
			return;
		}

		// Check each status register for requested service calls.
		for (std::size_t i = 0; i < statuses->size(); ++i) {
			if (statuses->operator[](i) == service_status::requested) execute(i);
		}

		// Read status registers again until started_ becomes false.
		if (started_) startReadCommandsTimer();
	});
}

bool RpcServer::execute(std::size_t index) {
	// Check busy flag.
	detail::RpcService & service = *services_[index];
	if (service.busy.test_and_set()) return false;

	// Execute service.
	std::uint8_t status_var = base_register_ + index;
	service.execute([this, &service, status_var] (DetailedError error) {
		// Handle error.
		if (error) on_error_(prefixError(std::move(error), "executing service " + service.name + ": "));

		// Always write status (also after error).
		WriteUint8Var command{status_var, error ? service_status::error : service_status::idle};
		client_->sendCommand(command, 100ms, [this, &service] (ErrorOr<void> result) {
			if (!result) on_error_(prefixError(std::move(result.error_unchecked()), "writing status for service " + service.name + ": "));
			service.busy.clear();
		});
	});
	return true;
}

}}
