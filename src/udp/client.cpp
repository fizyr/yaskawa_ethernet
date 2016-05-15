#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "send_command.hpp"
#include "../impl/connect.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

Client::Client(boost::asio::io_service & ios) : socket_(ios) {};

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::readByteVariable(ReadByteVariable::Request request, ResultCallback<ReadByteVariable::Response> const & callback) {
	impl::sendCommand<ReadByteVariable>(request, socket_, callback);
}

void Client::writeByteVariable(WriteByteVariable::Request request, ResultCallback<WriteByteVariable::Response> const & callback) {
	impl::sendCommand<WriteByteVariable>(request, socket_, callback);
}

}}}
