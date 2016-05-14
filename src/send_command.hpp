#pragma once
#include "decode.hpp"
#include "error.hpp"
#include "string_view.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

#include <cstdint>
#include <memory>
#include <utility>

namespace dr {
namespace yaskawa {

namespace impl {
	/// Parse a message from a streambuf and remove it from the buffer.
	/**
	 * The decoder must accept a single string_view and return the result.
	 * Exactly the amount of bytes specified as size are removed from the buffer.
	 * Hence, the size of the message to be parsed must be known already when calling this function.
	 */
	template<typename Decoder>
	auto parseFromBuffer(
		boost::asio::streambuf & buffer, ///< The buffer to parse the message from.
		std::size_t size,                ///< The size of the message (not of the buffer).
		Decoder && decoder               ///< The decoder.
	) -> decltype(std::forward<Decoder>(decoder)(std::declval<string_view &&>()))
	{
		auto result = std::forward<Decoder>(decoder)(string_view{boost::asio::buffer_cast<char const *>(buffer.data()), size});
		buffer.consume(size);
		return result;
	}

	/// Class representing a command session.
	/**
	 * The session consists of the following actions:
	 *   - Write command and data.
	 *   - Read command response.
	 *   - Read response data.
	 */
	template<typename Socket, typename Callback>
	class StartSession : public std::enable_shared_from_this<StartSession<Socket, Callback>> {
		using Ptr = std::shared_ptr<StartSession>;
		Socket * socket;
		boost::asio::streambuf * read_buffer;
		boost::asio::streambuf * write_buffer;

		Callback callback;

	public:
		/// Construct a command session.
		StartSession(Socket & socket, boost::asio::streambuf & read_buffer, boost::asio::streambuf & write_buffer, Callback callback) :
			socket(&socket),
			read_buffer(&read_buffer),
			write_buffer(&write_buffer),
			callback(callback) {}

		/// Start the session by writing the command.
		void start() {
			auto callback = std::bind(&StartSession::onWriteCommand, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_write(*socket, write_buffer->data(), callback);
		}

	protected:
		/// Called when the command and data have been written.
		void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			write_buffer->consume(bytes_transferred);
			if (error) callback(ErrorOr<std::string>(error));

			auto callback = std::bind(&StartSession::onReadResponse, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
		}

		/// Called when the command response has been read.
		void onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(ErrorOr<std::string>(error));
			ErrorOr<std::string> response = parseFromBuffer(*read_buffer, bytes_transferred, decodeResponse);
			callback(response);
		}
	};

	/// Class representing a command session.
	/**
	 * The session consists of the following actions:
	 *   - Write command and data.
	 *   - Read command response.
	 *   - Read response data.
	 */
	template<typename T, typename Socket, typename Decoder, typename Callback>
	class CommandSession : public std::enable_shared_from_this<CommandSession<T, Socket, Decoder, Callback>> {
		using Ptr = std::shared_ptr<CommandSession>;
		Socket * socket;
		boost::asio::streambuf * read_buffer;
		boost::asio::streambuf * write_buffer;

		Decoder decoder;
		Callback callback;

	public:
		/// Construct a command session.
		CommandSession(Socket & socket, boost::asio::streambuf & read_buffer, boost::asio::streambuf & write_buffer, Decoder decoder, Callback callback) :
			socket(&socket),
			read_buffer(&read_buffer),
			write_buffer(&write_buffer),
			decoder(decoder),
			callback(callback) {}

		/// Start the session by writing the command.
		void start() {
			auto callback = std::bind(&CommandSession::onWriteCommand, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_write(*socket, write_buffer->data(), callback);
		}

	protected:
		/// Called when the command and data have been written.
		void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			write_buffer->consume(bytes_transferred);
			if (error) callback(ErrorOr<T>(error));

			auto callback = std::bind(&CommandSession::onReadResponse, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
		}

		/// Called when the command response has been read.
		void onReadResponse(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(ErrorOr<T>(error));
			ErrorOr<std::string> response = parseFromBuffer(*read_buffer, bytes_transferred, decodeResponse);
			if (!response.valid()) return callback(response.errorDetails());

			auto callback = std::bind(&CommandSession::onReadData, this, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2);
			boost::asio::async_read_until(*socket, *read_buffer, ResponseMatcher{}, callback);
		}

		/// Called when the command data has been read.
		void onReadData(Ptr, boost::system::error_code const & error, std::size_t bytes_transferred) {
			if (error) callback(ErrorOr<T>(error));
			ErrorOr<T> response = parseFromBuffer(*read_buffer, bytes_transferred, decoder);
			callback(response);
		}
	};
}

/// Send a command and receive the reply asynchronously.
template<typename Socket, typename Callback>
void sendStartCommand(Socket & socket, boost::asio::streambuf & read_buffer, boost::asio::streambuf & write_buffer, Callback callback) {
	auto session = std::make_shared<impl::StartSession<Socket, Callback>>(socket, read_buffer, write_buffer, callback);
	session->start();
}

/// Send a command and receive the reply asynchronously.
template<typename T, typename Socket, typename Decoder, typename Callback>
void sendCommand(Socket & socket, boost::asio::streambuf & read_buffer, boost::asio::streambuf & write_buffer, Decoder decoder, Callback callback) {
	auto session = std::make_shared<impl::CommandSession<T, Socket, Decoder, Callback>>(socket, read_buffer, write_buffer, decoder, callback);
	session->start();
}

}}
