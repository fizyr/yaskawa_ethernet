#pragma once
#include <string>
#include <utility>

namespace dr {
namespace yaskawa {
namespace impl {

struct ResponseMatcher {
	using result_type = bool;

	enum class Type {
		unknown,
		crlf,
		cr,
	} type = Type::unknown;

	std::string prefix;
	bool cr = false;

	bool consume(char c) {
		if (type == Type::unknown) {
			if (c == '\r') {
				type = Type::cr;
				return true;
			}

			prefix.push_back(c);
			if (prefix == "OK:" || prefix == "NG:" || prefix == "ERROR:") {
				type = Type::crlf;
			} else if (prefix == "0000") {
				type = Type::crlf;
			} else if (prefix.size() >= 6) {
				type = Type::cr;
			}
		} else if (type == Type::crlf) {
			if (cr && c == '\n') return true;
			cr = (c == '\r');
		} else if (type == Type::cr) {
			if (c == '\r') return true;
		}
		return false;
	}

	template<typename Iterator>
	std::pair<Iterator, bool> operator() (Iterator begin, Iterator end) {
		for (Iterator i = begin; i != end; ++i) {
			if (consume(*i)) return std::make_pair(i + 1, true);
		}
		return std::make_pair(end, false);
	}
};

}}}
