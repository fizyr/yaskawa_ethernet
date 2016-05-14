#include "decode.hpp"
#include "error.hpp"

#include <algorithm>
#include <utility>

namespace dr {
namespace yaskawa {

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

	/// Discard leading spaces from a string view.
	string_view discardSpaces(string_view input) {
		char const * begin = input.data();
		char const * end   = begin + input.size();
		while (isSpace(*begin)) ++begin;
		return {begin, std::size_t(end - begin)};
	}

	/// Prase an integer from a string view.
	/**
	 * \return A pair holding the amount of parsed bytes and the parsed integer.
	 */
	std::pair<std::size_t, long int> parseInt(string_view input) {
		long int result = 0;
		char const * begin = input.data();

		bool negative = false;
		if (isSign(*begin)) {
			negative = *begin == '-';
			++begin;
		}

		while (isNumerical(*begin)) {
			result = result * 10 + (*begin - '0');
			++begin;
		}

		if (negative) result *= -1;

		return std::make_pair(begin - input.begin(), result);
	}
}

ErrorOr<std::string> decodeResponse(string_view message) {
	// The response is terminated by a CRLF, so ignore that bit.
	message = stripResponseFrame(message);

	if (startsWith(message, "NG:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return {boost::system::error_code{errc::command_failed}, std::string(start, message.end())};
	}

	if (startsWith(message, "OK:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return {std::string(start, message.end())};
	}

	return {boost::system::error_code{errc::malformed_response}, "response does not start with `NG:' or `OK:'"};
}

ErrorOr<std::uint8_t> decodeReadByteVariableData(string_view message) {
	std::size_t parsed;
	long int result;
	message = stripDataFrame(message);
	std::tie(parsed, result) = parseInt(discardSpaces(message));
	if (parsed == 0) return {boost::system::error_code{errc::malformed_response}, "response does not contain an integral value"};

	message.remove_prefix(parsed);
	message = discardSpaces(message);
	if (!message.empty()) return {boost::system::error_code{errc::malformed_response}, "response contains more data than expected"};

	if (result < 0)    return {boost::system::error_code{errc::malformed_response}, "received byte value (" + std::to_string(result) + ") is less than zero"};
	if (result > 0xff) return {boost::system::error_code{errc::malformed_response}, "received byte value (" + std::to_string(result) + ") exceeds 0xFF"};
	return uint8_t(result);
}


}}
