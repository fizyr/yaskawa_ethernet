#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "send_command.hpp"
#include "udp/commands.hpp"
#include "../impl/connect.hpp"

#include "encode.hpp"
#include "decode.hpp"

#include <atomic>
#include <memory>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

Client::Client(boost::asio::io_service & ios) :
	socket_(ios),
	read_buffer_{std::make_unique<std::array<std::uint8_t, 512>>()} {}

void Client::connect(std::string const & host, std::string const & port, std::chrono::milliseconds timeout, Callback callback) {
	auto on_connect = [this, callback = std::move(callback)] (DetailedError error) {
		onConnect(error, std::move(callback));
	};
	asyncResolveConnect({host, port}, timeout, socket_, on_connect);
}

void Client::connect(std::string const & host, std::uint16_t port, std::chrono::milliseconds timeout, Callback callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::close() {
	socket_.close();
}

Client::HandlerToken Client::registerHandler(std::uint8_t request_id, std::function<void(ResponseHeader const &, string_view)> handler) {
	auto result = requests_.insert({request_id, {std::chrono::steady_clock::now(), handler}});
	if (!result.second) throw std::logic_error("request_id " + std::to_string(request_id) + " is already taken, can not register handler");
	return result.first;
}

void Client::removeHandler(HandlerToken token) {
	requests_.erase(token);
}

namespace {
	/// Read a variable from a robot.
	template<typename T>
	void readVariable(
		Client & client,
		std::uint8_t request_id,
		int index,
		std::chrono::milliseconds timeout,
		std::function<void (ErrorOr<typename T::type>)> callback
	) {
		std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(0, T::command_single, index, 0, service::get_all, request_id));
		impl::sendCommand(client, request_id, std::move(message), decodeReadResponse<T>, timeout, std::move(callback));
	}

	/// Read multiple variables from a robot.
	template<typename T>
	void readVariables(
		Client & client,
		std::uint8_t request_id,
		int index,
		int count,
		std::chrono::milliseconds timeout,
		std::function<void (ErrorOr<std::vector<typename T::type>>)> callback
	) {
		std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(4, T::command_multiple, index, 0, service::read_multiple, request_id));
		writeLittleEndian<std::uint32_t>(message, count);
		impl::sendCommand(client, request_id, std::move(message), decodeReadMultipleResponse<T>, timeout, std::move(callback));
	}

	/// Write a variable to a robot.
	template<typename T>
	void writeVariable(
		Client & client,
		std::uint8_t request_id,
		int index,
		typename T::type const & value,
		std::chrono::milliseconds timeout,
		std::function<void (ErrorOr<void>)> callback
	) {
		std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(T::encoded_size, T::command_single, index, 0, service::set_all, request_id));
		T::encode(message, value);
		impl::sendCommand(client, request_id, std::move(message), decodeWriteResponse<T>, timeout, std::move(callback));
	}

	/// Write multiple variables to a robot.
	template<typename T>
	void writeVariables(
		Client & client,
		std::uint8_t request_id,
		int index,
		std::vector<typename T::type> const & values,
		std::chrono::milliseconds timeout,
		std::function<void (ErrorOr<void>)> callback
	) {
		std::vector<std::uint8_t> message = encodeRequestHeader(makeRobotRequestHeader(4 + values.size() * T::encoded_size, T::command_multiple, index, 0, service::write_multiple, request_id));
		writeLittleEndian<std::uint32_t>(message, values.size());
		for (auto const & value : values) T::encode(message, value);
		impl::sendCommand(client, request_id, std::move(message), decodeWriteResponse<T>, timeout, std::move(callback));
	}
}

// Byte variables.

void Client::readByte(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::uint8_t>)> callback) {
	readVariable<ByteVariable>(*this, request_id_++, index, timeout, std::move(callback));
}

void Client::readBytes(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::uint8_t>>)> callback) {
	readVariables<ByteVariable>(*this, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeByte(int index, std::uint8_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<ByteVariable>(*this, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeBytes(int index, std::vector<std::uint8_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<ByteVariable>(*this, request_id_++, index, values, timeout, std::move(callback));
}

// Integer (16 bit) variables.

void Client::readInt16(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int16_t>)> callback) {
	readVariable<Int16Variable>(*this, request_id_++, index, timeout, std::move(callback));
}

void Client::readInt16s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::int16_t>>)> callback) {
	readVariables<Int16Variable>(*this, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeInt16(int index, std::int16_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int16Variable>(*this, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeInt16s(int index, std::vector<std::int16_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Int16Variable>(*this, request_id_++, index, values, timeout, std::move(callback));
}

// Integer (32 bit) variables.

void Client::readInt32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::int32_t>)> callback) {
	readVariable<Int32Variable>(*this, request_id_++, index, timeout, std::move(callback));
}

void Client::readInt32s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<std::int32_t>>)> callback) {
	readVariables<Int32Variable>(*this, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeInt32(int index, std::int32_t value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Int32Variable>(*this, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeInt32s(int index, std::vector<std::int32_t> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Int32Variable>(*this, request_id_++, index, values, timeout, std::move(callback));
}

// Float (32 bit) variables.

void Client::readFloat32(int index, std::chrono::milliseconds timeout, std::function<void (ErrorOr<float>)> callback) {
	readVariable<Float32Variable>(*this, request_id_++, index, timeout, std::move(callback));
}

void Client::readFloat32s(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<float>>)> callback) {
	readVariables<Float32Variable>(*this, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeFloat32(int index, float value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<Float32Variable>(*this, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeFloat32s(int index, std::vector<float> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<Float32Variable>(*this, request_id_++, index, values, timeout, std::move(callback));
}

// Robot position variables.

void Client::readRobotPosition(int index, std::chrono::milliseconds timeout, std::function<void(ErrorOr<Position>)> callback) {
	readVariable<PositionVariable>(*this, request_id_++, index, timeout, std::move(callback));
}

void Client::readRobotPositions(int index, int count, std::chrono::milliseconds timeout, std::function<void (ErrorOr<std::vector<Position>>)> callback) {
	readVariables<PositionVariable>(*this, request_id_++, index, count, timeout, std::move(callback));
}

void Client::writeRobotPosition(int index, Position value, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariable<PositionVariable>(*this, request_id_++, index, value, timeout, std::move(callback));
}

void Client::writeRobotPositions(int index, std::vector<Position> const & values, std::chrono::milliseconds timeout, std::function<void(ErrorOr<void>)> callback) {
	writeVariables<PositionVariable>(*this, request_id_++, index, values, timeout, std::move(callback));
}

void Client::onConnect(DetailedError error, Callback callback) {
	callback(error);
	if (!error) receive();
}

void Client::receive() {
	auto callback = std::bind(&Client::onReceive, this, std::placeholders::_1, std::placeholders::_2);
	socket_.async_receive(boost::asio::buffer(read_buffer_->data(), read_buffer_->size()), callback);
}

void Client::onReceive(boost::system::error_code error, std::size_t message_size) {
	if (error) {
		if (on_error) on_error(make_error_code(std::errc(error.value())));
		receive();
		return;
	}

	// Decode the response header.
	string_view message{reinterpret_cast<char const *>(read_buffer_->data()), message_size};
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) {
		if (on_error) on_error(header.error());
		receive();
		return;
	}

	// Find the right handler for the response.
	auto handler = requests_.find(header->request_id);
	if (handler == requests_.end()) {
		if (on_error) on_error({std::errc::invalid_argument, "no handler for request id " + std::to_string(header->request_id)});
		receive();
		return;
	}

	// Erase and invoke the handler.
	auto callback = std::move(handler->second.on_reply);
	callback(*header, message);
	receive();
}

}}}
