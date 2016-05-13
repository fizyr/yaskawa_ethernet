#include "decode.hpp"
#include "error.hpp"

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

ErrorOr<Response> decodeResponse(string_view message) {
	// The response is terminated by a CRLF, so ignore that bit.
	message = stripResponseFrame(message);

	if (startsWith(message, "NG:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return Response{false, std::string(start, message.end())};
	}

	if (startsWith(message, "OK:"_v)) {
		char const * start = std::find_if_not(message.begin() + 3, message.end(), isSpace);
		return Response{true, std::string(start, message.end())};
	}

	return boost::system::error_code{errc::malformed_response};
}



}}
