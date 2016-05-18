#include "tcp/protocol.hpp"

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

	/// Discard leading spaces from a string view.
	string_view discardSpaces(string_view input) {
		char const * begin = input.data();
		char const * end   = begin + input.size();
		while (isSpace(*begin)) ++begin;
		return {begin, std::size_t(end - begin)};
	}
}

}}}
