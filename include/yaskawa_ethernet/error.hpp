#pragma once
#include "impl/error.hpp"

#include <string>
#include <system_error>
#include <type_traits>

namespace dr {
namespace yaskawa {

/// Get a reference to the Yaskawa error category.
impl::YaskawaCategory const & yaskawa_category();

/// Protocol error code constants.
namespace errc {
	enum errc_t {
		malformed_response  = 0x01,
		command_failed      = 0x02,
	};

	inline std::error_code      make_error_code(errc_t code)      { return {code, yaskawa_category()}; }
	inline std::error_condition make_error_condition(errc_t code) { return {code, yaskawa_category()}; }
}

/// Enum type for protocol error codes.
using errc_t = errc::errc_t;

}}

namespace std {
	template<> class is_error_code_enum     <dr::yaskawa::errc_t> : std::true_type {};
	template<> class is_error_condition_enum<dr::yaskawa::errc_t> : std::true_type {};
}
