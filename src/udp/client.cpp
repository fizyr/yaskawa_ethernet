#include "../connect.hpp"
#include "./read_file.hpp"
#include "./write_file.hpp"

#include "commands.hpp"
#include "udp/client.hpp"
#include "udp/message.hpp"
#include "udp/protocol.hpp"

#include <atomic>
#include <memory>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {

Client::Client(asio::io_service & ios) :
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

// File control.

void Client::readFileList(
	std::string type,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<std::vector<std::string>>)> on_done,
	std::function<void(std::size_t bytes_received)> on_progress
) {
	impl::readFile(*this, request_id_++, ReadFileList{std::move(type)}, timeout, std::move(on_done), std::move(on_progress));
}

void Client::readFile(
	std::string name,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<std::string>)> on_done,
	std::function<void(std::size_t bytes_received)> on_progress
) {
	impl::readFile(*this, request_id_++, ReadFile{std::move(name)}, timeout, std::move(on_done), std::move(on_progress));
}

void Client::writeFile(
	std::string name,
	std::string data,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<void>)> on_done,
	std::function<void(std::size_t bytes_sent, std::size_t total_bytes)> on_progress
) {
	impl::writeFile(*this, request_id_++, WriteFile{std::move(name), std::move(data)}, timeout, std::move(on_done), std::move(on_progress));
}

void Client::deleteFile(
	std::string name,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<void>)> on_done
) {
	sendCommand(DeleteFile{std::move(name)}, timeout, on_done);
}

// Other stuff

void Client::onConnect(DetailedError error, Callback callback) {
	callback(error);
	if (!error) receive();
}

void Client::receive() {
	// Make sure we stop reading if the socket is closed.
	// Otherwise in rare cases we can miss an operation_canceled and continue reading forever.
	if (!socket_.is_open()) return;
	auto callback = std::bind(&Client::onReceive, this, std::placeholders::_1, std::placeholders::_2);
	socket_.async_receive(asio::buffer(read_buffer_->data(), read_buffer_->size()), callback);
}

void Client::onReceive(std::error_code error, std::size_t message_size) {
	if (error == std::errc::operation_canceled) return;
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
		if (on_error) on_error({errc::unknown_request, "no handler for request id " + std::to_string(header->request_id)});
		receive();
		return;
	}

	// Invoke the handler (a copy, so it can erase itself safely).
	auto callback = handler->second.on_reply;
	callback(*header, message);
	receive();
}

}}}
