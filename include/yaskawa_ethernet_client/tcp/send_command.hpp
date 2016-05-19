#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "tcp/protocol.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

#include <cstdint>
#include <memory>
#include <utility>
#include <ostream>

namespace dr {
namespace yaskawa {
namespace tcp {

namespace impl {
	/// Class representing a command session.
	/**
	 * The session consists of the following actions:
	 * - Write command and data.
	 * - Read command response.
	 * - Read response data.
	 */
	template<typename Command, typename Socket, typename Callback, bool ReadData>
	class CommandSession : public std::enable_shared_from_this<CommandSession<Command, Socket, Callback, ReadData>> {
		using Ptr = std::shared_ptr<CommandSession>;
		using Request  = typename Command::Request;
		using Response = typename Command::Response;

		Socket * socket;
		boost::asio::streambuf * read_buffer;
		boost::asio::streambuf command_buffer;
		boost::asio::streambuf data_buffer;

		Callback callback;

	public:
		/// Construct a command session.
		CommandSession(Socket & socket, boost::asio::streambuf & read_buffer, Callback callback) :
			socket(&socket),
			read_buffer(&read_buffer),
			callback(callback) {}

		/// Start the session by writing the command.
		void start(Request const & request) {
			encode(command_buffer, data_buffer, request);
			auto callback = std::bind(&CommandSession::onWriteCommand, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_write(*socket, command_buffer.data(), callback);
		}

	protected:
		/// Called when the command has been written.
		void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			command_buffer.consume(bytes_transferred);
			if (error) callback(ErrorOr<Response>(error));

			auto callback = std::bind(&CommandSession::onReadResponse<ReadData>, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
		}

		/// Called when the command response has been read.
		/**
		 * If the response type is CommandResponse, don't read data, just invoke the callback.
		 */
		template<bool R2 = ReadData>
		typename std::enable_if<R2>::type
		onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(error);
			ErrorOr<CommandResponse> decoded = decode<CommandResponse>(string_view{boost::asio::buffer_cast<char const *>(read_buffer->data()), bytes_transferred});
			read_buffer->consume(bytes_transferred);
			return callback(decoded);
		}

		/// Called when the command response has been read.
		/**
		 * If the response type is not CommandResponse, read the data response too.
		 */
		template<bool R2 = ReadData>
		typename std::enable_if<not R2>::type
		onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(error);
			ErrorOr<CommandResponse> decoded = decode<CommandResponse>(string_view{boost::asio::buffer_cast<char const *>(read_buffer->data()), bytes_transferred});
			read_buffer->consume(bytes_transferred);
			if (!decoded.valid()) return callback(decoded.error());

			// If the command has data, write it.
			if (data_buffer.size()) {
				auto callback = std::bind(&CommandSession::onWriteData, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
				boost::asio::async_write(*socket, data_buffer.data(), callback);

			// Otherwise read the response directly.
			} else {
				auto callback = std::bind(&CommandSession::onReadData, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
				boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
			}
		}

		/// Called when the command data has been written.
		void onWriteData(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(error);
			data_buffer.consume(bytes_transferred);

			auto callback = std::bind(&CommandSession::onReadData, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
		}

		/// Called when the command data has been read.
		void onReadData(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(ErrorOr<Response>(error));
			ErrorOr<Response> result = decode<Response>(string_view{boost::asio::buffer_cast<char const *>(read_buffer->data()), bytes_transferred});
			read_buffer->consume(bytes_transferred);
			callback(std::move(result));
		}
	};
}

/// Send the command currently in the the write buffer and receive the reply asynchronously.
/**
 * Sending a command includes three phases:
 * - Write command and data.
 * - Read command response.
 * - Read response data.
 */
template<typename Command, typename Socket, typename Callback>
void sendCommand(typename Command::Request const & request, Socket & socket, boost::asio::streambuf & read_buffer, Callback callback) {
	constexpr bool ReadData = std::is_same<typename Command::Response, CommandResponse>::value;
	auto session = std::make_shared<impl::CommandSession<Command, Socket, Callback, ReadData>>(socket, read_buffer, callback);
	session->start(request);
}

}}}
