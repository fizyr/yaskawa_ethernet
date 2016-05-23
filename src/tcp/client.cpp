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

Client::Client(boost::asio::io_service & ios) : socket_(ios) {}

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::start(int keep_alive, ResultCallback<CommandResponse> callback) {
	sendCommand<StartCommand>({keep_alive}, socket_, read_buffer_, callback);
}

void Client::readInt8Variable(int index, ResultCallback<std::uint8_t> callback) {
	sendCommand<ReadInt8Variable>({index}, socket_, read_buffer_, callback);
}
void Client::readInt16Variable(int index, ResultCallback<std::int16_t> callback) {
	sendCommand<ReadInt16Variable>({index}, socket_, read_buffer_, callback);
}
void Client::readInt32Variable(int index, ResultCallback<std::int32_t> callback) {
	sendCommand<ReadInt32Variable>({index}, socket_, read_buffer_, callback);
}
void Client::readFloat32Variable(int index, ResultCallback<float> callback) {
	sendCommand<ReadFloat32Variable>({index}, socket_, read_buffer_, callback);
}
void Client::readPositionVariable(int index, ResultCallback<Position> callback) {
	sendCommand<ReadPositionVariable>({index}, socket_, read_buffer_, callback);
}

void Client::writeInt8Variable (int index, std::uint8_t value, ResultCallback<void> callback) {
	sendCommand<WriteInt8Variable>({index, value}, socket_, read_buffer_, callback);
}
void Client::writeInt16Variable(int index, std::int16_t value, ResultCallback<void> callback) {
	sendCommand<WriteInt16Variable>({index, value}, socket_, read_buffer_, callback);
}
void Client::writeInt32Variable(int index, std::int32_t value, ResultCallback<void> callback) {
	sendCommand<WriteInt32Variable>({index, value}, socket_, read_buffer_, callback);
}
void Client::writeFloat32Variable(int index, float value, ResultCallback<void> callback) {
	sendCommand<WriteFloat32Variable>({index, value}, socket_, read_buffer_, callback);
}
void Client::writePositionVariable(int index, Position value, ResultCallback<void> callback) {
	sendCommand<WritePositionVariable>({index, value}, socket_, read_buffer_, callback);
}

}}}
