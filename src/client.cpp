#include "client.hpp"
#include "encode.hpp"
#include "decode.hpp"
#include "connect.hpp"
#include "send_command.hpp"

#include <memory>
#include <atomic>
#include <utility>

namespace dr {
namespace yaskawa {

EthernetClient::EthernetClient(boost::asio::io_service & ios) : socket_(ios) {};

void EthernetClient::connect(std::string const & host, std::string const & port, unsigned int timeout, Callback const & callback) {
	asyncResolveConnect({host, port}, timeout, socket_, callback);
}

void EthernetClient::connect(std::string const & host, std::uint16_t port, unsigned int timeout, Callback const & callback) {
	connect(host, std::to_string(port), timeout, callback);
}

void EthernetClient::start(int keep_alive, ResultCallback<std::string> const & callback) {
	encodeStartRequest(std::ostream(&write_buffer_), keep_alive);
	sendStartCommand(socket_, read_buffer_, write_buffer_, callback);

}

}}
