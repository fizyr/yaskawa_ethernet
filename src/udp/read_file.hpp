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

#include <iostream>

namespace dr {
namespace yaskawa {
namespace udp {
namespace impl {

class ReadFileSession : public std::enable_shared_from_this<ReadFileSession> {
	using Ptr = std::shared_ptr<ReadFileSession>;
	using DoneCallback     = std::function<void(ErrorOr<std::string>)>;
	using ProgressCallback = std::function<void(std::size_t bytes_received)>;

public:
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
	std::size_t blocks_received_ = 0;

public:
	/// Construct a command session.
	ReadFileSession(
		Client & client,
		std::uint8_t request_id,
		string_view file_name,
		std::chrono::milliseconds timeout,
		DoneCallback on_done,
		ProgressCallback on_progress = nullptr
	) :
		client_(&client),
		request_id_{request_id},
		timer_(client.ios()),
		timeout_{timeout},
		on_done_(std::move(on_done)),
		on_progress_(std::move(on_progress))
	{
		write_buffer_ = encodeRequestHeader(makeFileRequestHeader(file_name.size(), commands::file::read_file, request_id));
		ReadFile::encode(write_buffer_, file_name);
		std::cout << "command buffer size: " << this << " " << write_buffer_.size() << "\n";
	}

	/// Start the session by writing the command and starting the timeout..
	void start() {
		std::cout << "write buffer size: " << this << " " << write_buffer_.size() << "\n";
		auto response_callback = std::bind(&ReadFileSession::onResponse, this, self(), std::placeholders::_1, std::placeholders::_2);
		handler_ = client_->registerHandler(request_id_, response_callback);

		std::cout << "write buffer size: "  << this << " "<< write_buffer_.size() << "\n";

		auto callback = std::bind(&ReadFileSession::onWriteCommand, this, self(), std::placeholders::_1, std::placeholders::_2);
		client_->socket().async_send(boost::asio::buffer(write_buffer_.data(), write_buffer_.size()), callback);
		resetTimeout();
	}

protected:
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

	/// Write an ack for a data block.
	void writeAck(std::uint32_t block_number) {
		std::vector<std::uint8_t> write_buffer_ = encodeRequestHeader(makeFileRequestHeader(0, commands::file::read_file, request_id_, block_number, true));
		auto callback = std::bind(&ReadFileSession::onWriteAck, this, self(), std::placeholders::_1, std::placeholders::_2);
		client_->socket().async_send(boost::asio::buffer(write_buffer_.data(), write_buffer_.size()), callback);
	}

	/// Called when an ack has been written.
	void onWriteAck(Ptr, boost::system::error_code const & error, std::size_t) {
		if (done_.load()) return;
		if (error && done_.exchange(true) == false) {
			timer_.cancel();
			client_->removeHandler(handler_);
			on_done_(DetailedError(std::errc(error.value())));
			return;
		}
	}

	/// Called when the command response has been read.
	void onResponse(Ptr, ResponseHeader const & header, string_view data) {
		if (header.status != 0) return stopSession(commandFailed(header.status, header.extra_status));
		std::size_t block = header.block_number & 0x7fffffff;
		bool done = header.block_number & 0x80000000;
		if (block != blocks_received_ + 1) {
			return stopSession(DetailedError{std::errc::invalid_argument, "unexpected block number, got " + std::to_string(block) + ", expected " + std::to_string(blocks_received_ + 1)});
		}

		std::copy(data.begin(), data.end(), std::back_inserter(file_data_));

		if (on_progress_) on_progress_(file_data_.size());
		if (done) {
			stopSession(file_data_);
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

	void stopSession(ErrorOr<std::string> result) {
		if (done_.exchange(true)) return;
		timer_.cancel();
		client_->removeHandler(handler_);
		return on_done_(result);
	}
};

void readFile(
	Client & client,
	std::uint8_t request_id,
	string_view name,
	std::chrono::milliseconds timeout,
	std::function<void(ErrorOr<std::string>)> on_done,
	std::function<void(std::size_t bytes_received)> on_progress
) {
	auto session = std::make_shared<ReadFileSession>(
		client,
		request_id,
		name,
		timeout,
		std::move(on_done),
		std::move(on_progress)
	);
	std::cout << "write buffer size: " << session.get() << " " << session->write_buffer_.size() << "\n";
	session->start();
}

}}}}
