#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "encode.hpp"
#include "decode.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>

#include <algorithm>
#include <atomic>
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
class WriteFileSession : public std::enable_shared_from_this<WriteFileSession> {
	using Ptr = std::shared_ptr<WriteFileSession>;
	using DoneCallback     = std::function<void(ErrorOr<void>)>;
	using ProgressCallback = std::function<void(std::size_t bytes_written, std::size_t total_bytes)>;

	Client * client_;
	std::uint8_t request_id_;
	Client::HandlerToken handler_;
	boost::asio::steady_timer timer_;
	std::chrono::milliseconds timeout_;
	std::vector<std::uint8_t> write_buffer_;
	std::string file_data_;

	DoneCallback on_done_;
	ProgressCallback on_progress_;

	std::atomic_bool done_{false};
	bool command_sent_ = false;
	std::size_t blocks_sent_ = 0;

public:
	/// Construct a command session.
	WriteFileSession(
		Client & client,
		std::uint8_t request_id,
		string_view file_name,
		std::string file_data,
		std::chrono::milliseconds timeout,
		DoneCallback on_done,
		ProgressCallback on_progress = nullptr
	) :
		client_(&client),
		request_id_{request_id},
		timer_(client.ios()),
		timeout_{timeout},
		file_data_{std::move(file_data)},
		on_done_(std::move(on_done)),
		on_progress_(std::move(on_progress))
	{
		write_buffer_ = encodeRequestHeader(makeFileRequestHeader(file_name.size(), commands::file::write_file, request_id));
		WriteFile::encode(write_buffer_, file_name);
	}

	/// Start the session by writing the command and starting the timeout..
	void start() {
		auto response_callback = std::bind(&WriteFileSession::onResponse, this, self(), std::placeholders::_1, std::placeholders::_2);
		handler_ = client_->registerHandler(request_id_, response_callback);

		auto callback = std::bind(&WriteFileSession::onWriteCommand, this, self(), std::placeholders::_1, std::placeholders::_2);
		client_->socket().async_send(boost::asio::buffer(write_buffer_.data(), write_buffer_.size()), callback);
		resetTimeout();
	}

protected:
	std::size_t bytesSent() const {
		return std::min(file_data_.size(), blocks_sent_ * max_payload_size);
	}

	/// Get a shared pointer to this session.
	Ptr self() { return this->shared_from_this(); }

	/// Called when the message has been written.
	void onWriteCommand(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done_.load()) return;
		if (error && done_.exchange(true) == false) {
			timer_.cancel();
			client_->removeHandler(handler_);
			on_done_(DetailedError(std::errc(error.value())));
		}
		command_sent_ = true;
	}

	void writeNextBlock() {
		std::size_t bytes_sent = blocks_sent_ * max_payload_size;
		std::size_t remaining  = file_data_.size() - bytes_sent;
		std::size_t block_size = std::min(remaining, max_payload_size);
		char const * start = file_data_.data() + bytes_sent;
		std::uint32_t block_number = blocks_sent_ + 1;
		if (bytes_sent + block_size == file_data_.size()) block_number |= 0x8000000;
		std::vector<std::uint8_t> message = encodeRequestHeader(makeFileRequestHeader(block_size, commands::file::write_file, request_id_, blocks_sent_ + 1));
		FileData::encode(message, {start, block_size});
	}

	/// Called when a data block has been written.
	void onWriteBlock(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done_.load()) return;
		if (error && done_.exchange(true) == false) {
			timer_.cancel();
			client_->removeHandler(handler_);
			on_done_(DetailedError(std::errc(error.value())));
			return;
		}
		++blocks_sent_;
	}

	/// Called when the command response has been read.
	void onResponse(Ptr, ResponseHeader const & header, string_view) {
		if (header.status != 0) {
			if (done_.exchange(true)) return;
			timer_.cancel();
			client_->removeHandler(handler_);
			return on_done_(commandFailed(header.status, header.extra_status));
		}
		if (on_progress_) on_progress_(bytesSent(), file_data_.size());
		if (bytesSent() >= file_data_.size()) {
			if (done_.exchange(true)) return;
			timer_.cancel();
			client_->removeHandler(handler_);
			on_done_(in_place_valid);
			return;
		}
	}

	void resetTimeout() {
		timer_.expires_from_now(timeout_);
		timer_.async_wait([this, self=self()] (boost::system::error_code error) {
			if (done_.exchange(true)) return;
			client_->removeHandler(handler_);
			if (error) return on_done_(DetailedError(std::errc(error.value()), "waiting for reply to request " + std::to_string(request_id_)));
			on_done_(DetailedError(std::errc::timed_out, "waiting for reply to request " + std::to_string(request_id_)));
		});
	}
};

void writeFile(
	Client & client,
	std::uint8_t request_id,
	string_view name,
	std::string data,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<void>)> on_done,
	std::function<void(std::size_t bytes_sent, std::size_t total_bytes)> on_progress
) {
	auto session = std::make_shared<WriteFileSession>(
		client,
		request_id,
		name,
		std::move(data),
		timeout,
		std::move(on_done),
		std::move(on_progress)
	);
	session->start();
}

}}}}
