#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"
#include "../impl/response_matcher.hpp"

#include <ostream>

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

/**
 * An Encoder specialization must have the following members:
 * - ErrorOr<T> operator() (string_view buffer);
 */
template<typename T>
struct Encoder;

/**
 * A Decoder specialization must have the following members:
 * - void encodeRequest(std::ostream & out, T request);
 */
template<typename T>
struct Decoder;

template<typename T>
void encode(std::ostream & out, T && input) {
	Encoder<typename std::decay<T>::type>{}(out, std::forward<T>(input));
}

template<typename T>
void encode(std::ostream && out, T && input) {
	encode<T>(out, std::forward<T>(input));
}

template<typename T>
ErrorOr<T> decode(string_view buffer) {
	return Decoder<T>{}(buffer);
}

namespace impl {
	template<typename T>
	struct Encoder {
		void operator() (std::ostream & stream, T const &);
	};

	template<typename T>
	struct Decoder {
		ErrorOr<T> operator() (string_view message);
	};
}

template<> struct Encoder<StartCommand::Request> : impl::Encoder<StartCommand::Request> {};
template<> struct Decoder<CommandResponse>       : impl::Decoder<CommandResponse> {};

template<> struct Encoder<ReadInt8Variable::Request>     : impl::Encoder<ReadInt8Variable::Request>     {};
template<> struct Encoder<ReadInt16Variable::Request>    : impl::Encoder<ReadInt16Variable::Request>    {};
template<> struct Encoder<ReadInt32Variable::Request>    : impl::Encoder<ReadInt32Variable::Request>    {};
template<> struct Encoder<ReadFloat32Variable::Request>  : impl::Encoder<ReadFloat32Variable::Request>  {};
template<> struct Decoder<ReadInt8Variable::Response>    : impl::Decoder<ReadInt8Variable::Response>    {};
template<> struct Decoder<ReadInt16Variable::Response>   : impl::Decoder<ReadInt16Variable::Response>   {};
template<> struct Decoder<ReadInt32Variable::Response>   : impl::Decoder<ReadInt32Variable::Response>   {};
template<> struct Decoder<ReadFloat32Variable::Response> : impl::Decoder<ReadFloat32Variable::Response> {};

template<> struct Encoder<WriteInt8Variable::Request>     : impl::Encoder<WriteInt8Variable::Request>     {};
template<> struct Encoder<WriteInt16Variable::Request>    : impl::Encoder<WriteInt16Variable::Request>    {};
template<> struct Encoder<WriteInt32Variable::Request>    : impl::Encoder<WriteInt32Variable::Request>    {};
template<> struct Encoder<WriteFloat32Variable::Request>  : impl::Encoder<WriteFloat32Variable::Request>  {};
template<> struct Decoder<WriteInt8Variable::Response>    : impl::Decoder<WriteInt8Variable::Response>    {};
template<> struct Decoder<WriteInt16Variable::Response>   : impl::Decoder<WriteInt16Variable::Response>   {};
template<> struct Decoder<WriteInt32Variable::Response>   : impl::Decoder<WriteInt32Variable::Response>   {};
template<> struct Decoder<WriteFloat32Variable::Response> : impl::Decoder<WriteFloat32Variable::Response> {};


}}}
