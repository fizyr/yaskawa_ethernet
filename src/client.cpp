#include "client.hpp"
#include "encode.hpp"
#include "decode.hpp"
#include "connect.hpp"

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

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

	auto write_handler = [this, callback] (boost::system::error_code const & error, std::size_t bytes_transferred) {
		if (error) return callback(error);
		write_buffer_.consume(bytes_transferred);

		auto read_handler = [this, callback] (boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) return callback(error);
			string_view data = {boost::asio::buffer_cast<char const *>(read_buffer_.data()), bytes_transferred};
			callback(decodeResponse(data));
		};
		boost::asio::async_read_until(socket_, read_buffer_, ResponseMatcher{}, read_handler);
	};

	boost::asio::async_write(socket_, write_buffer_.data(), write_handler);
}

}}
