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

Client::Client(boost::asio::io_service & ios) : socket_(ios) {}

void Client::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void Client::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void Client::readByteVariable(ReadInt8Variable::Request request, unsigned int timeout, ResultCallback<ReadInt8Variable::Response> const & callback) {
	impl::sendCommand<ReadInt8Variable>(request, timeout, request_id_++, socket_, callback);
}

void Client::writeByteVariable(WriteInt8Variable::Request request, unsigned int timeout, ResultCallback<WriteInt8Variable::Response> const & callback) {
	impl::sendCommand<WriteInt8Variable>(request, timeout, request_id_++, socket_, callback);
}

}}}
