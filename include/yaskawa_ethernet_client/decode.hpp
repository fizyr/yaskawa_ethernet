#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "impl/response_matcher.hpp"

#include <string>

namespace dr {
namespace yaskawa {

/// Default constructible function object type that matches response messages.
/**
 * For use with boost::asio::read_until.
 */
using ResponseMatcher = impl::ResponseMatcher;

/// Decode a response message.
ErrorOr<std::string> decodeResponse(string_view message);

/// Decode an empty data response.
ErrorOr<void> decodeEmptyData(string_view message);

/// Decode a read byte variable response.
ErrorOr<std::uint8_t> decodeReadByteVariableData(string_view message);

}}
