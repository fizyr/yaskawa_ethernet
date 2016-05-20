#include "decode.hpp"
#include "tcp/protocol.hpp"

#include <algorithm>
#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace tcp {

template<>
ErrorOr<CommandResponse> decode<CommandResponse>(string_view message) {
	message = stripResponseFrame(message);

	if (startsWith(message, "NG: "_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return DetailedError{errc::command_failed, std::string(start, message.end())};
	}

	if (startsWith(message, "OK: "_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return CommandResponse{std::string(start, message.end())};
	}

	return DetailedError{errc::malformed_response, "response does not start with `NG:' or `OK:'"};
}

template<>
ErrorOr<void> decode<void>(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	if (message != "0000\r\n"_v) return {boost::system::error_code{errc::malformed_response}, "expected empty response, received something else"};
	return ErrorOr<void>{};
}

template<>
ErrorOr<ReadInt8Variable::Response> decode<ReadInt8Variable::Response>(string_view message) {
	return decodeIntMessage<std::uint8_t>(message);
}

template<>
ErrorOr<ReadInt16Variable::Response> decode<ReadInt16Variable::Response>(string_view message) {
	return decodeIntMessage<std::uint16_t>(message);
}

template<>
ErrorOr<ReadInt32Variable::Response> decode<ReadInt32Variable::Response>(string_view message) {
	return decodeIntMessage<std::uint32_t>(message);
}

template<>
ErrorOr<ReadFloat32Variable::Response> decode<ReadFloat32Variable::Response>(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() != 1) return wrongArgCount(params.size(), 1);
	return parseFloat<float>(params[0]);
}

template<>
ErrorOr<ReadPositionVariable::Response> decode<ReadPositionVariable::Response>(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() < 8 && params.size() > 9) return wrongArgCount(params.size(), 8, 9);
	return decodePosition(array_view<string_view>{params});
}

}}}
