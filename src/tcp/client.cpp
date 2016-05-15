#include "tcp/client.hpp"
#include "tcp/encode.hpp"
#include "tcp/decode.hpp"
#include "../impl/connect.hpp"
#include "send_command.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {
namespace tcp {

Client::Client(boost::asio::io_service & ios) : socket_(ios) {};

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::start(int keep_alive, ResultCallback<std::string> const & callback) {
	encodeStartRequest(std::ostream(&write_buffer_), keep_alive);
	sendStartRequest(socket_, read_buffer_, write_buffer_, callback);
}

void Client::readByteVariable(int index, ResultCallback<std::uint8_t> const & callback) {
	encodeReadVariable(std::ostream(&write_buffer_), VariableType::byte_type, index);
	sendCommand<std::uint8_t>(socket_, read_buffer_, write_buffer_, decodeReadByteVariableData, callback);
}

void Client::writeByteVariable(int index, std::uint8_t value, ResultCallback<void> const & callback) {
	encodeWriteByteVariable(std::ostream(&write_buffer_), index, value);
	sendCommand<void>(socket_, read_buffer_, write_buffer_, decodeEmptyData, callback);
}

}}}
