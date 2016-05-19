#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"
#include "impl/response_matcher.hpp"

#include <boost/asio/streambuf.hpp>

namespace dr {
namespace yaskawa {
namespace tcp {

struct CommandResponse {
	std::string message;

	CommandResponse() = default;
	CommandResponse(std::string const & message) : message(message) {};
	CommandResponse(std::string      && message) : message(std::move(message)) {};

	operator std::string const & () const & { return message; }
	operator std::string       & ()       & { return message; }
	operator std::string      && ()      && { return std::move(message); }
};

struct StartCommand {
	struct Request  { int keep_alive; };
	using Response = CommandResponse;
};

/// Default constructible function object type that matches response messages.
/**
 * For use with boost::asio::read_until.
 */
using ResponseMatcher = impl::ResponseMatcher;

template<typename T>
void encode(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, T message);

template<typename T>
void encode(boost::asio::streambuf && stream, T && message) {
	encode(stream, std::forward<T>(message));
}

template<typename T>
ErrorOr<T> decode(string_view data);

template<> void encode<StartCommand::Request>(boost::asio::streambuf &, boost::asio::streambuf &, StartCommand::Request);
template<> ErrorOr<CommandResponse> decode<CommandResponse>(string_view);
template<> ErrorOr<void>            decode<void>(string_view);

template<> void encode<ReadInt8Variable::Request>   (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, ReadInt8Variable::Request request);
template<> void encode<ReadInt16Variable::Request>  (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, ReadInt16Variable::Request request);
template<> void encode<ReadInt32Variable::Request>  (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, ReadInt32Variable::Request request);
template<> void encode<ReadFloat32Variable::Request>(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, ReadFloat32Variable::Request request);
template<> ErrorOr<ReadInt8Variable::Response>    decode<ReadInt8Variable::Response>(string_view);
template<> ErrorOr<ReadInt16Variable::Response>   decode<ReadInt16Variable::Response>(string_view);
template<> ErrorOr<ReadInt32Variable::Response>   decode<ReadInt32Variable::Response>(string_view);
template<> ErrorOr<ReadFloat32Variable::Response> decode<ReadFloat32Variable::Response>(string_view);

template<> void encode<WriteInt8Variable::Request>   (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, WriteInt8Variable::Request);
template<> void encode<WriteInt16Variable::Request>  (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, WriteInt16Variable::Request);
template<> void encode<WriteInt32Variable::Request>  (boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, WriteInt32Variable::Request);
template<> void encode<WriteFloat32Variable::Request>(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, WriteFloat32Variable::Request);
template<> ErrorOr<WriteInt8Variable::Response>     decode<WriteInt8Variable::Response>(string_view data);
template<> ErrorOr<WriteInt16Variable::Response>    decode<WriteInt16Variable::Response>(string_view data);
template<> ErrorOr<WriteInt32Variable::Response>    decode<WriteInt32Variable::Response>(string_view data);
template<> ErrorOr<WriteFloat32Variable::Response>  decode<WriteFloat32Variable::Response>(string_view data);


}}}
