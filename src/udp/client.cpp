#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "send_command.hpp"
#include "udp/commands.hpp"
#include "../impl/connect.hpp"

#include "encode.hpp"
#include "decode.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

Client::Client(boost::asio::io_service & ios) : socket_(ios) {}

void Client::connect(std::string const & host, std::string const & port, std::chrono::milliseconds timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, std::chrono::milliseconds timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::close() {
	socket_.close();
}

/// Read a variable from a robot.
template<typename T>
void readVariable(
	Client::Socket & socket,
	std::uint8_t request_id,
	int index,
	std::chrono::milliseconds timeout,
	std::function<void (ErrorOr<typename T::type>)> callback
) {
	std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(0, T::command_single, index, 0, service::get_all, request_id));
	impl::sendCommand(std::move(message), decodeReadResponse<T>, timeout, socket, std::move(callback));
}

/// Read multiple variables from a robot.
template<typename T>
void readVariables(
	Client::Socket & socket,
	std::uint8_t request_id,
	int index,
	int count,
	std::chrono::milliseconds timeout,
	std::function<void (ErrorOr<std::vector<typename T::type>>)> callback
) {
	std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(4, T::command_multiple, index, 0, service::read_multiple, request_id));
	writeLittleEndian<std::uint32_t>(message, count);
	impl::sendCommand(std::move(message), decodeReadMultipleResponse<T>, timeout, socket, std::move(callback));
}

/// Write a variable to a robot.
template<typename T>
void writeVariable(
	Client::Socket & socket,
	std::uint8_t request_id,
	int index,
	typename T::type const & value,
	std::chrono::milliseconds timeout,
	std::function<void (ErrorOr<void>)> callback
) {
	std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(T::encoded_size, T::command_single, index, 0, service::set_all, request_id));
	T::encode(message, value);
	impl::sendCommand(std::move(message), decodeWriteResponse<T>, timeout, socket, std::move(callback));
}

/// Write multiple variables to a robot.
template<typename T>
void writeVariables(
	Client::Socket & socket,
	std::uint8_t request_id,
	int index,
	std::vector<typename T::type> const & values,
	std::chrono::milliseconds timeout,
	std::function<void (ErrorOr<void>)> callback
) {
	std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(4 + values.size() * T::encoded_size, T::command_multiple, index, 0, service::write_multiple, request_id));
	writeLittleEndian<std::uint32_t>(message, values.size());
	for (auto const & value : values) T::encode(message, value);
	impl::sendCommand(std::move(message), decodeWriteResponse<T>, timeout, socket, std::move(callback));
}

// Byte variables.

void Client::readByte(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::uint8_t>)> callback) {
	readVariable<ByteVariable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::readBytes(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::uint8_t>> const &)> callback) {
	readVariables<ByteVariable>(socket_, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeByte(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<ByteVariable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeBytes(int index, std::vector<std::uint8_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<ByteVariable>(socket_, request_id_++, index, values, timeout, std::move(callback));
}

// Integer (16 bit) variables.

void Client::readInt16(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int16_t>)> callback) {
	readVariable<Int16Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::readInt16s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::int16_t>> const &)> callback) {
	readVariables<Int16Variable>(socket_, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeInt16(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int16Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeInt16s(int index, std::vector<std::int16_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Int16Variable>(socket_, request_id_++, index, values, timeout, std::move(callback));
}

// Integer (32 bit) variables.

void Client::readInt32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int32_t>)> callback) {
	readVariable<Int32Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::readInt32s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::int32_t>> const &)> callback) {
	readVariables<Int32Variable>(socket_, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeInt32(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int32Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeInt32s(int index, std::vector<std::int32_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Int32Variable>(socket_, request_id_++, index, values, timeout, std::move(callback));
}

// Float (32 bit) variables.

void Client::readFloat32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<float>)> callback) {
	readVariable<Float32Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::readFloat32s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<float>> const &)> callback) {
	readVariables<Float32Variable>(socket_, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeFloat32(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Float32Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeFloat32s(int index, std::vector<float> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Float32Variable>(socket_, request_id_++, index, values, timeout, std::move(callback));
}

// Robot position variables.

void Client::readRobotPosition(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback) {
	readVariable<PositionVariable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::readRobotPositions(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<Position>> const &)> callback) {
	readVariables<PositionVariable>(socket_, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeRobotPosition(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<PositionVariable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeRobotPositions(int index, std::vector<Position> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<PositionVariable>(socket_, request_id_++, index, values, timeout, std::move(callback));
}

}}}
