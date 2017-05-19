#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "send_command.hpp"
#include "udp/commands.hpp"
#include "../impl/connect.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

Client::Client(boost::asio::io_service & ios) : socket_(ios) {}

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::close() {
	socket_.close();
}

// Byte variables.

void Client::readByteVariable(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::uint8_t>)> callback) {
	impl::sendCommand(
		encodeReadVariable(
			request_id_++,
			index,
			commands::robot::readwrite_int8_variable
		),
		decodeReadInt8Variable, timeout, socket_, std::move(callback)
	);
}

void Client::writeByteVariable(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	impl::sendCommand(
		encodeWriteInt8Variable(request_id_++, index, value),
		decodeWriteVariable, timeout, socket_, callback
	);
}

// Integer (16 bit) variables.

void Client::readInt16Variable(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int16_t>)> callback) {
	impl::sendCommand(
		encodeReadVariable(
			request_id_++,
			index,
			commands::robot::readwrite_int16_variable
		),
		decodeReadInt16Variable, timeout, socket_, std::move(callback)
	);
}

void Client::writeInt16Variable(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	impl::sendCommand(
		encodeWriteInt16Variable(request_id_++, index, value),
		decodeWriteVariable, timeout, socket_, std::move(callback)
	);
}

// Integer (32 bit) variables.

void Client::readInt32Variable(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int32_t>)> callback) {
	impl::sendCommand(
		encodeReadVariable(
			request_id_++,
			index,
			commands::robot::readwrite_int32_variable
		),
		decodeReadInt32Variable, timeout, socket_, std::move(callback)
	);
}

void Client::writeInt32Variable(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	impl::sendCommand(
		encodeWriteInt32Variable(request_id_++, index, value),
		decodeWriteVariable, timeout, socket_, std::move(callback)
	);
}

// Float (32 bit) variables.

void Client::readFloat32Variable(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<float>)> callback) {
	impl::sendCommand(
		encodeReadVariable(
			request_id_++,
			index,
			commands::robot::readwrite_float_variable
		),
		decodeReadFloat32Variable, timeout, socket_, std::move(callback)
	);
}

void Client::writeFloat32Variable(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	impl::sendCommand(
		encodeWriteFloat32Variable(request_id_++, index, value),
		decodeWriteVariable, timeout, socket_, std::move(callback)
	);
}

// Robot position variables.

void Client::readRobotPositionVariable(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback) {
	impl::sendCommand(
		encodeReadVariable(
			request_id_++,
			index,
			commands::robot::readwrite_robot_position_variable
		),
		decodeReadPositionVariable, timeout, socket_, std::move(callback)
	);
}

void Client::writeRobotPositionVariable(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	impl::sendCommand(
		encodeWritePositionVariable(request_id_++, index, value, commands::robot::readwrite_robot_position_variable),
		decodeWriteVariable, timeout, socket_, std::move(callback)
	);

}

}}}
