#include "decode.hpp"
#include <boost/utility/string_ref.hpp>
#include <algorithm>

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
}

char const * findMessageEnd(string_view message) {
	char const * cr = std::find(message.begin(), message.end(), '\r');
	bool         lf = cr + 1 < message.end() && *(cr + 1) == '\n';
	if (cr == message.end()) return message.end();

	if (startsWith(message, "0000\r\n"_v))  return message.begin() + 6;
	if (startsWith(message, "OK:"_v) && lf) return cr + 2;
	if (startsWith(message, "NG:"_v) && lf) return cr + 2;

	// If not one of the previous messages, this is a DATA response.
	// DATA responses are terminated by a single CR, no LF.
	return cr + 1;
}

Response decodeResponse(string_view message) {
	// The response is terminated by a CRLF, so ignore that bit.
	message = stripResponseFrame(message);

	if (message.size() < 3) throw std::runtime_error("Invalid response. Reponse too small.");

	if (startsWith(message, "NG:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return {false, std::string(start, message.end())};
	}

	if (startsWith(message, "OK:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return {true, std::string(start, message.end())};
	}

	throw std::runtime_error("Unknown response.");
}



}}
