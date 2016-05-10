#include "string_view.hpp"

#include <string>

namespace dr {
namespace yaskawa {

struct Response {
	bool success;
	std::string message;

	explicit operator bool() const { return success; }
};

/// Find the end of a message in a buffer.
/**
 * Return a pointer past the end of the message (including terminator).
 */
char const * findMessageEnd(string_view message);

/// Decode a response message.
Response decodeResponse(string_view message);

}}
