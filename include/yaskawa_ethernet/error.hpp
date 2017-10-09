#pragma once
#include <dr_error/error_or.hpp>

#include <string>
#include <system_error>
#include <type_traits>

namespace dr {
namespace yaskawa {

/// Get a reference to the Yaskawa error category.
std::error_category const & yaskawa_category();

/// Protocol error code constants.
namespace errc {
	enum errc_t {
		malformed_response  = 0x01,
		command_failed      = 0x02,
		unknown_request     = 0x03,
	};

	inline std::error_code      make_error_code(errc_t code)      { return {code, yaskawa_category()}; }
	inline std::error_condition make_error_condition(errc_t code) { return {code, yaskawa_category()}; }
}

/// Enum type for protocol error codes.
using errc_t = errc::errc_t;

DetailedError malformedResponse(std::string message);
DetailedError commandFailed(std::uint16_t status, std::uint16_t extra_status);

DetailedError expectValue(std::string name, int value, int expected);
DetailedError expectValueMin(std::string name, int value, int min);
DetailedError expectValueMax(std::string name, int value, int max);
DetailedError expectValueMinMax(std::string name, int value, int min, int max);
DetailedError expectSize(std::string description, std::size_t actual_size, std::size_t expected_size);
DetailedError expectSizeMin(std::string description, std::size_t actual_size, std::size_t minimum_size);
DetailedError expectSizeMax(std::string description, std::size_t actual_size, std::size_t maximum_size);
DetailedError expectSizeMinMax(std::string description, std::size_t actual_size, std::size_t maximum_size);

}}

namespace std {
	template<> class is_error_code_enum     <dr::yaskawa::errc_t> : std::true_type {};
	template<> class is_error_condition_enum<dr::yaskawa::errc_t> : std::true_type {};
}
