#include "send_command.hpp"
#include "tcp/client.hpp"
#include "tcp/protocol.hpp"
#include "../impl/connect.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {
namespace tcp {

namespace {
	template<typename Encoder, typename Decoder, typename Callback, typename Socket, typename ...Args>
	void sendCommand(Encoder && encoder, Decoder && decoder, Callback && callback, Socket & socket, boost::asio::streambuf & read_buffer, Args && ...args) {
		auto session = makeCommandSesssion(std::forward<Decoder>(decoder), std::forward<Callback>(callback), socket, read_buffer);
		std::forward<Encoder>(encoder)(session->command_buffer, session->data_buffer, std::forward<Args>(args)...);
		session->send();
	}
}

Client::Client(boost::asio::io_service & ios) : socket_(ios) {}

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

// Start commands.

void Client::start(int keep_alive, ResultCallback<std::string> callback) {
	auto session = makeStartCommandSesssion(callback, socket_, read_buffer_);
	encodeStartCommand(session->command_buffer, keep_alive);
	session->send();
}

void Client::enableServo(bool enable, ResultCallback<void> callback) {
	sendCommand(encodeServoOn, decodeEmptyData, callback, socket_, read_buffer_, enable);
}

void Client::startJob(std::string const & job, ResultCallback<void> callback) {
	sendCommand(encodeStartJob, decodeEmptyData, callback, socket_, read_buffer_, job);
}

// Read position.

void Client::readPulsePosition(ResultCallback<PulsePosition> callback) {
	sendCommand(encodeReadPulsePosition, decodeReadPulsePosition, callback, socket_, read_buffer_);
}
void Client::readCartesianPosition(CoordinateSystem system, ResultCallback<CartesianPosition> callback) {
	auto wrapped = [callback, system] (ErrorOr<CartesianPosition> && position) {
		if (position) {
			position->frame() = system;
		}
		callback(position);
	};
	sendCommand(encodeReadCartesianPosition, decodeReadCartesianPosition, wrapped, socket_, read_buffer_, system);
}

// Read / write IO.

void Client::readIo(unsigned int start, unsigned int count, ResultCallback<std::vector<std::uint8_t>> callback) {
	sendCommand(encodeReadIo, decodeReadIo, callback, socket_, read_buffer_, start, count);
}
void Client::writeIo(unsigned int start, std::vector<std::uint8_t> const & data, ResultCallback<void> callback) {
	sendCommand(encodeWriteIo, decodeEmptyData, callback, socket_, read_buffer_, start, data);
}

// Read variables.

void Client::readByteVariable(int index, ResultCallback<std::uint8_t> callback) {
	sendCommand(encodeReadByteVariable, decodeReadByteVariable, callback, socket_, read_buffer_, index);
}
void Client::readIntVariable(int index, ResultCallback<std::int16_t> callback) {
	sendCommand(encodeReadIntVariable, decodeReadIntVariable, callback, socket_, read_buffer_, index);
}
void Client::readDoubleIntVariable(int index, ResultCallback<std::int32_t> callback) {
	sendCommand(encodeReadDoubleIntVariable, decodeReadDoubleIntVariable, callback, socket_, read_buffer_, index);
}
void Client::readRealVariable(int index, ResultCallback<float> callback) {
	sendCommand(encodeReadRealVariable, decodeReadRealVariable, callback, socket_, read_buffer_, index);
}
void Client::readPositionVariable(int index, ResultCallback<Position> callback) {
	sendCommand(encodeReadPositionVariable, decodeReadPositionVariable, callback, socket_, read_buffer_, index);
}

// Write variables.

void Client::writeByteVariable (int index, std::uint8_t value, ResultCallback<void> callback) {
	sendCommand(encodeWriteByteVariable, decodeEmptyData, callback, socket_, read_buffer_, index, value);
}
void Client::writeIntVariable(int index, std::int16_t value, ResultCallback<void> callback) {
	sendCommand(encodeWriteIntVariable, decodeEmptyData, callback, socket_, read_buffer_, index, value);
}
void Client::writeDoubleIntVariable(int index, std::int32_t value, ResultCallback<void> callback) {
	sendCommand(encodeWriteDoubleIntVariable, decodeEmptyData, callback, socket_, read_buffer_, index, value);
}
void Client::writeRealVariable(int index, float value, ResultCallback<void> callback) {
	sendCommand(encodeWriteRealVariable, decodeEmptyData, callback, socket_, read_buffer_, index, value);
}
void Client::writePositionVariable(int index, Position value, ResultCallback<void> callback) {
	sendCommand(encodeWritePositionVariable, decodeEmptyData, callback, socket_, read_buffer_, index, value);
}

}}}
