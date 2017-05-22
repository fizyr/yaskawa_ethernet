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

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::close() {
	socket_.close();
}

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

// Byte variables.

void Client::readByte(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::uint8_t>)> callback) {
	readVariable<ByteVariable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::writeByte(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<ByteVariable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

// Integer (16 bit) variables.

void Client::readInt16(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int16_t>)> callback) {
	readVariable<Int16Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::writeInt16(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int16Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

// Integer (32 bit) variables.

void Client::readInt32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int32_t>)> callback) {
	readVariable<Int32Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::writeInt32(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int32Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

// Float (32 bit) variables.

void Client::readFloat32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<float>)> callback) {
	readVariable<Float32Variable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::writeFloat32(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Float32Variable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

// Robot position variables.

void Client::readRobotPosition(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback) {
	readVariable<PositionVariable>(socket_, request_id_++, index, timeout, std::move(callback));
}

void Client::writeRobotPosition(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<PositionVariable>(socket_, request_id_++, index, value, timeout, std::move(callback));
}

}}}
