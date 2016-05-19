#include "tcp/protocol.hpp"
#include "error.hpp"

#include <algorithm>
#include <utility>
#include <vector>
#include <cstdint>
#include <cmath>

namespace dr {
namespace yaskawa {
namespace tcp {

namespace {
	/// Make a string_view from a string literal.
	constexpr string_view operator "" _v (char const * data, std::size_t size) {
		return string_view{data, size};
	}

	/// Check if a character is a space.
	bool isSpace(char x) {
		return x == ' ';
	}

	/// Check if a character is numerical.
	bool isNumerical(char x) {
		return x >= '0' && x <= '9';
	}

	/// Check if a character is a positive or negative sign.
	bool isSign(char x) {
		return x == '+' || x == '-';
	}

	/// Check if a string starts with a given prefix.
	bool startsWith(string_view string, string_view prefix) {
		if (string.size() < prefix.size()) return false;
		return std::equal(prefix.begin(), prefix.end(), string.begin());
	}

	/// Strip the framing (a trailing CRLF) from a regular response message.
	string_view stripResponseFrame(string_view message) {
		return {message.begin(), message.size() - 2};
	}

	/// Strip the framing (a trailing CR) from a data response message.
	string_view stripDataFrame(string_view message) {
		return {message.begin(), message.size() - 1};
	}

	/// Split a string view in whitespace seperated parameters.
	std::vector<string_view> splitData(string_view data) {
		std::vector<string_view> result;

		char const * begin = data.begin();
		while (begin < data.end()) {
			char const * end = std::find(begin, data.end(), ',');
			begin = std::find_if_not(begin, data.end(), isSpace);
			result.emplace_back(begin, end - begin);
			if (begin == data.end()) break;
			begin = end + 1;
		}

		return result;
	}

	DetailedError malformedResponse(std::string && message) {
		return DetailedError{errc::malformed_response, std::move(message)};
	}

	DetailedError wrongArgCount(int actual, int expected) {
		return malformedResponse("received " + std::to_string(actual) + " data components, expected " + std::to_string(expected));
	}

	template<typename T>
	ErrorOr<T> parseInt(string_view data, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
		if (data.empty()) return malformedResponse("empty integer value received");
		long long result = 0;
		int sign = 1;

		if (isSign(data[0])) {
			sign = data[0] == '-' ? -1 : 1;
			data.remove_prefix(1);
		}

		while (!data.empty()) {
			char c = data[0];
			data.remove_prefix(1);
			if (!isNumerical(c)) {
				return malformedResponse(std::string("invalid character encountered in integral value: `") + c + "' (" + std::to_string(c) + ")");
			}
			result = result * 10 + (c - '0');
		}
		result *= sign;
		if (result < min) return malformedResponse("received value (" + std::to_string(result) + ") exceeds the lowest allowed value (" + std::to_string(min) + ")");
		if (result > max) return malformedResponse("received value (" + std::to_string(result) + ") exceeds the highest allowed value (" + std::to_string(min) + ")");
		return result;
	}

	template<typename T>
	ErrorOr<T> parseFloat(string_view data, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
		if (data.empty()) return malformedResponse("empty floating point value received");
		long double result = 0;
		int sign = 1;

		if (isSign(data[0])) {
			sign = data[0] == '-' ? -1 : 1;
			data.remove_prefix(1);
		}

		// Integral parts.
		while (!data.empty()) {
			char c = data[0];
			data.remove_prefix(1);
			if (c == '.') break;
			if (!isNumerical(c)) {
				return malformedResponse(std::string("invalid character encountered in integral value: `") + c + "' (" + std::to_string(c) + ")");
			}
			result = result * 10 + (c - '0');
		}

		// Fractional part.
		for (int position = 1; !data.empty(); ++position) {
			char c = data[0];
			data.remove_prefix(1);
			if (!isNumerical(c)) {
				return malformedResponse(std::string("invalid character encountered in integral value: `") + c + "' (" + std::to_string(c) + ")");
			}
			result += (c - '0') * std::pow(10, -position);
		}

		result *= sign;
		if (result < min) return malformedResponse("received value (" + std::to_string(result) + ") exceeds the lowest allowed value (" + std::to_string(min) + ")");
		if (result > max) return malformedResponse("received value (" + std::to_string(result) + ") exceeds the highest allowed value (" + std::to_string(min) + ")");
		return result;
	}

	template<typename T>
	ErrorOr<T> decodeIntMessage(string_view message) {
		std::vector<string_view> params = splitData(stripDataFrame(message));
		if (params.size() != 1) return wrongArgCount(params.size(), 1);
		return parseInt<T>(params[0]);
	}
}


template<>
ErrorOr<CommandResponse> decode<CommandResponse>(string_view message) {
	message = stripResponseFrame(message);

	if (startsWith(message, "NG:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return DetailedError{errc::command_failed, std::string(start, message.end())};
	}

	if (startsWith(message, "OK:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return CommandResponse{std::string(start, message.end())};
	}

	return DetailedError{errc::malformed_response, "response does not start with `NG:' or `OK:'"};
}

template<>
ErrorOr<void> decode<void>(string_view message) {
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
	std::vector<string_view> params = splitData(stripDataFrame(message));
	if (params.size() != 1) return wrongArgCount(params.size(), 1);
	return parseFloat<float>(params[0]);
}

}}}
