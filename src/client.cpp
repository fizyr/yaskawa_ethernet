#include "client.hpp"
#include "encode.hpp"
#include "decode.hpp"

#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <memory>

namespace dr {
namespace yaskawa {

EthernetClient::EthernetClient(boost::asio::io_service & ios) : socket(ios) {};

void EthernetClient::start(int keep_alive, Callback const & callback) {
	auto buffer = std::make_shared<boost::asio::streambuf>();
	std::ostream stream(buffer.get());
	encodeStartRequest(stream, keep_alive);

	auto handler = [this, callback, buffer] (boost::system::error_code const & error, std::size_t bytes_transferred) {
		if (error) return callback(error);
		buffer->consume(bytes_transferred);

		auto handler = [this, callback, buffer] (boost::system::error_code const & error, std::size_t bytes_transferred) {
			
		};
		boost::asio::async_read_until(socket, *buffer, ResponseMatcher{}, handler);
	};

	boost::asio::async_write(socket, buffer->data(), handler);
}

}}
