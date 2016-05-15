#pragma once
#include <cstdint>

namespace dr {
namespace yaskawa {

struct ReadByteVariable {
	struct Request {
		int index;
	};

	struct Response {
		std::uint8_t value;
	};
};

struct WriteByteVariable {
	struct Request {
		int index;
		std::uint8_t value;
	};

	struct Response {};
};

}}
