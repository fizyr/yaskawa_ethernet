#pragma once
#include "error.hpp"
#include "string_view.hpp"
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
#include <system_error>
#include <utility>

#include <iostream>

namespace dr {
namespace yaskawa {
namespace udp {
namespace impl {

template<typename Command>
class ReadFileSession : public std::enable_shared_from_this<ReadFileSession<Command>> {
	using Response         = typename Command::Response;
	using DoneCallback     = std::function<void(ErrorOr<Response>)>;
	using ProgressCallback = std::function<void(std::size_t bytes_received)>;

public:
	Client * client_;
	std::uint8_t request_id_;
	Command command_;
	Client::HandlerToken handler_;
	asio::steady_timer timer_;
	std::chrono::milliseconds timeout_;
	std::vector<std::uint8_t> write_buffer_;
	std::string read_buffer_;

	DoneCallback on_done_;
	ProgressCallback on_progress_;

	std::atomic_bool done_{false};
	std::size_t blocks_received_ = 0;

public:
	/// Construct a command session.
	ReadFileSession(
		Client & client,
		std::uint8_t request_id,
		Command command,
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
	{
		read_buffer_.reserve(1024);

		// Encode the command.
		encode(write_buffer_, request_id, command_);
	}

	void start() {
		// Register the response handler.
		handler_ = client_->registerHandler(request_id_, [this, self = self()] (ResponseHeader const & header, string_view data) {
			onResponse(header, data);
		});

		// Send the command.
		client_->socket().async_send(asio::buffer(write_buffer_.data(), write_buffer_.size()), [this, self = self()] (std::error_code error, std::size_t) {
			if (error) return stopSession(DetailedError(error, "writing command for request " + std::to_string(request_id_)));
		});

		// Start the timeout.
		resetTimeout();
	}

protected:
	/// Get a shared pointer to this session.
	std::shared_ptr<ReadFileSession> self() { return this->shared_from_this(); }

	/// Write an ack for a data block.
	void writeAck(std::uint32_t block_number) {
		auto buffer = std::make_shared<std::vector<std::uint8_t>>();
		encode(*buffer, makeFileRequestHeader(0, commands::file::read_file, request_id_, block_number, true));
		client_->socket().async_send(asio::buffer(*buffer), [this, self = self(), buffer = std::move(buffer)] (std::error_code error, std::size_t) {
			if (error) return stopSession(DetailedError(error, "writing ack for request " + std::to_string(request_id_)));
		});
	}

	/// Called when the command response has been read.
	void onResponse(ResponseHeader const & header, string_view data) {
		if (done_.load()) return;
		if (header.status != 0) return stopSession(commandFailed(header.status, header.extra_status));

		std::size_t block = header.block_number & 0x7fffffff;
		bool last_block = header.block_number & 0x80000000;

		if (auto error = checkValue("block number", block, blocks_received_ + 1)) return stopSession(error);
		writeAck(block);

		read_buffer_.insert(read_buffer_.end(), data.begin(), data.end());

		if (on_progress_) on_progress_(read_buffer_.size());
		if (last_block) stopSession(decode(header, std::move(read_buffer_), command_));
	}

	void resetTimeout() {
		timer_.expires_from_now(timeout_);
		timer_.async_wait([this, self = self()] (std::error_code error) {
			if (error == asio::error::operation_aborted) return;
			if (error) return stopSession(DetailedError(error, "waiting for reply to request " + std::to_string(request_id_)));
			stopSession(DetailedError(std::errc::timed_out, "waiting for reply to request " + std::to_string(request_id_)));
		});
	}

	void stopSession(ErrorOr<Response> result) {
		if (done_.exchange(true)) return;
		timer_.cancel();
		client_->removeHandler(handler_);
		return on_done_(result);
	}
};

template<typename Command>
void readFile(
	Client & client,
	std::uint8_t request_id,
	Command && command,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<typename Command::Response>)> on_done,
	std::function<void(std::size_t bytes_received)> on_progress
) {
	auto session = std::make_shared<ReadFileSession<std::decay_t<Command>>>(
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
