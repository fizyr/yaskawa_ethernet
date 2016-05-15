#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "udp/protocol.hpp"

#include <boost/asio/streambuf.hpp>
#include <boost/system/error_code.hpp>
#include <boost/container/vector.hpp>

#include <cstdint>
#include <memory>
#include <utility>

namespace dr {
namespace yaskawa {
namespace udp {
namespace impl {

/// Class representing a command session.
/**
 * The session consists of the following actions:
 * - Write command and data.
 * - Read command response.
 * - Read response data.
 */
template<typename Command, typename Socket, typename Callback>
class CommandSession : public std::enable_shared_from_this<CommandSession<Command, Socket, Callback>> {
	using Request  = typename Command::Request;
	using Response = typename Command::Response;
	using Ptr = std::shared_ptr<CommandSession>;
	Socket * socket;
	std::vector<std::uint8_t> write_buffer;
	boost::container::vector<std::uint8_t> read_buffer;

	Callback callback;

public:
	/// Construct a command session.
	CommandSession(Socket & socket, Callback && callback) :
		socket(&socket),
		callback(std::move(callback)) {}

	/// Start the session by writing the command.
	void start(Request const & request) {
		write_buffer = encode(request);
		auto callback = std::bind(&CommandSession::onWriteCommand, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
		socket->async_send(boost::asio::buffer(write_buffer.data(), write_buffer.size()), callback);
	}

protected:
	/// Called when the command and data have been written.
	void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t) {
		write_buffer.clear();
		if (error) callback(ErrorOr<Response>(error));

		read_buffer.resize(512, boost::container::default_init);
		auto callback = std::bind(&CommandSession::onReadResponse, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
		socket->async_receive(boost::asio::buffer(read_buffer.data(), read_buffer.size()), callback);
	}

	/// Called when the command response has been read.
	void onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
		read_buffer.resize(bytes_transferred);
		if (error) callback(ErrorOr<Response>(error));
		ErrorOr<Response> response = decode<Response>(string_view{reinterpret_cast<char *>(read_buffer.data()), read_buffer.size()});
		callback(response);
	}
};

template<typename Command, typename Socket, typename Callback>
void sendCommand(typename Command::Request const & request, Socket & socket, Callback && callback) {
	auto session = std::make_shared<impl::CommandSession<Command, Socket, Callback>>(socket, std::forward<Callback>(callback));
	session->start(request);
}

}}}}
