#include "decode.hpp"
#include "tcp/protocol.hpp"

#include <algorithm>
#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace tcp {

ErrorOr<std::string> decodeCommandResponse(string_view message) {
	message = stripResponseFrame(message);

	if (startsWith(message, "NG: "_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return DetailedError{errc::command_failed, std::string(start, message.end())};
	}

	if (startsWith(message, "OK: "_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return std::string{start, message.end()};
	}

	return DetailedError{errc::malformed_response, "response does not start with `NG:' or `OK:'"};
}

ErrorOr<void> decodeEmptyData(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	if (message != "0000\r\n"_v) return {boost::system::error_code{errc::malformed_response}, "expected empty response, received something else"};
	return ErrorOr<void>{};
}

ErrorOr<PulsePosition> decodeReadPulsePosition(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() < 12 && params.size() > 13) return wrongArgCount(params.size(), 12, 13);
	return decodePulsePosition({params.data(), 6u + unsigned(params.size() > 12)}, false);
}

ErrorOr<CartesianPosition> decodeReadCartesianPosition(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;
	std::vector<string_view> params = splitData(stripDataFrame(message));
	return decodeCartesianPosition(array_view<string_view>{params}, false);
}

ErrorOr<std::vector<std::uint8_t>> decodeReadIo(string_view message) {
	std::vector<string_view> data = splitData(stripDataFrame(message));
	std::vector<std::uint8_t> result;
	result.reserve(data.size());
	for (string_view param : data) {
		ErrorOr<std::uint8_t> parsed = parseInt<std::uint8_t>(param);
		if (!parsed.valid()) return parsed.error();
		result.push_back(parsed.get());
	}
	return result;
}

ErrorOr<std::uint8_t> decodeReadByteVariable(string_view message) {
	return decodeIntMessage<std::uint8_t>(message);
}

ErrorOr<std::int16_t> decodeReadIntVariable(string_view message) {
	return decodeIntMessage<std::uint16_t>(message);
}

ErrorOr<std::int32_t> decodeReadDoubleIntVariable(string_view message) {
	return decodeIntMessage<std::uint32_t>(message);
}

ErrorOr<float> decodeReadRealVariable(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() != 1) return wrongArgCount(params.size(), 1);
	return parseFloat<float>(params[0]);
}

ErrorOr<Position> decodeReadPositionVariable(string_view message) {
	DetailedError error = parseErrorMessage(message);
	if (error) return error;

	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() < 8 && params.size() > 9) return wrongArgCount(params.size(), 8, 9);
	return decodePosition(array_view<string_view>{params});
}

}}}
