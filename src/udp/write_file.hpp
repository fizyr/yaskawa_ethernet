#pragma once
#include "error.hpp"
#include "udp/client.hpp"
#include "udp/protocol.hpp"
#include "encode.hpp"
#include "decode.hpp"

#include <asio/steady_timer.hpp>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <system_error>
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
template<typename Command>
class WriteFileSession : public std::enable_shared_from_this<WriteFileSession<Command>> {
	using DoneCallback     = std::function<void(Result<void>)>;
	using ProgressCallback = std::function<void(std::size_t bytes_written, std::size_t total_bytes)>;

	Client * client_;
	std::uint8_t request_id_;
	WriteFile command_;
	Client::HandlerToken handler_;
	asio::steady_timer timer_;
	std::chrono::milliseconds timeout_;
	std::vector<std::uint8_t> write_buffer_;

	DoneCallback on_done_;
	ProgressCallback on_progress_;

	std::atomic_bool done_{false};
	std::size_t blocks_sent_ = 0;

public:
	/// Construct a command session.
	WriteFileSession(
		Client & client,
		std::uint8_t request_id,
		WriteFile command,
		std::chrono::milliseconds timeout,
		DoneCallback on_done,
		ProgressCallback on_progress = nullptr
	) :
		client_(&client),
		request_id_{request_id},
		command_{std::move(command)},
		timer_(client.ios()),
		timeout_{timeout},
		on_done_(std::move(on_done)),
		on_progress_(std::move(on_progress))
	{}

	void start() {
		// Encode the command.
		encode(write_buffer_, request_id_, command_);

		// Register the response handler.
		handler_ = client_->registerHandler(request_id_, [this, self = self()] (ResponseHeader const & header, std::string_view data) {
			onResponse(header, data);
		});

		// Send the command.
		client_->socket().async_send(asio::buffer(write_buffer_.data(), write_buffer_.size()), [this, self = self()] (std::error_code error, std::size_t) {
			if (done_.load()) return;
			if (error) return stopSession(Error{error, "writing command for request " + std::to_string(request_id_)});
		});

		// Start the timeout.
		resetTimeout();
	}

protected:
	std::size_t bytesSent() const {
		return std::min(blocks_sent_ * max_payload_size, command_.data.size());
	}

	/// Get a shared pointer to this session.
	std::shared_ptr<WriteFileSession> self() { return this->shared_from_this(); }

	void writeNextBlock() {
		std::size_t bytes_sent = blocks_sent_ * max_payload_size;
		std::size_t remaining  = command_.data.size() - bytes_sent;
		std::size_t block_size = std::min(remaining, max_payload_size);
		char const * start = command_.data.data() + bytes_sent;
		std::uint32_t block_number = blocks_sent_ + 1;
		if (bytes_sent + block_size == command_.data.size()) block_number |= 0x8000000;

		auto buffer = std::make_shared<std::vector<std::uint8_t>>();
		encode(*buffer, makeFileRequestHeader(block_size, commands::file::write_file, request_id_, blocks_sent_ + 1));

		std::array<asio::const_buffer, 2> buffer_list{{
			asio::buffer(*buffer),
			asio::buffer(start, block_size)
		}};

		client_->socket().async_send(buffer_list, [this, self = self(), buffer = std::move(buffer)] (std::error_code error, std::size_t) {
			if (done_.load()) return;
			if (error) return stopSession(Error{error, "writing block for request " + std::to_string(request_id_)});
		});
		++blocks_sent_;
	}

	/// Called when the a response has been received.
	void onResponse(ResponseHeader const & header, std::string_view data) {
		if (done_.load()) return;
		if (header.status != 0) return stopSession(commandFailed(header.status, header.extra_status));
		if (auto error = expectSize("response data", data.size(), 0)) return stopSession(error);

		// TODO: Implement retransmission logic.
		if (auto error = expectValue("ack", header.ack, true)) return stopSession(error);
		if (auto error = expectValue("block number", header.block_number, blocks_sent_)) return stopSession(error);

		if (on_progress_) on_progress_(bytesSent(), command_.data.size());
		if (bytesSent() >= command_.data.size()) stopSession(estd::in_place_valid);
	}

	void resetTimeout() {
		timer_.expires_from_now(timeout_);
		timer_.async_wait([this, self=self()] (std::error_code error) {
			if (error == asio::error::operation_aborted) return;
			if (error) return stopSession(Error(error, "waiting for reply to request " + std::to_string(request_id_)));
			stopSession(Error(std::errc::timed_out, "waiting for reply to request " + std::to_string(request_id_)));
		});
	}

	void stopSession(Result<void> result) {
		if (done_.exchange(true)) return;
		timer_.cancel();
		client_->removeHandler(handler_);
		return on_done_(result);
	}
};

template<typename Command>
void writeFile(
	Client & client,
	std::uint8_t request_id,
	Command command,
	std::chrono::milliseconds timeout,
	std::function<void(Result<void>)> on_done,
	std::function<void(std::size_t bytes_sent, std::size_t total_bytes)> on_progress
) {
	auto session = std::make_shared<WriteFileSession<std::decay_t<Command>>>(
		client,
		request_id,
		std::forward<Command>(command),
		timeout,
		std::move(on_done),
		std::move(on_progress)
	);
	session->start();
}

}}}}
