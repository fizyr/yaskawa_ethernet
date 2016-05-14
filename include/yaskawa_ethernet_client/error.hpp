#pragma once
#include "impl/error.hpp"

#include <boost/system/error_code.hpp>

#include <system_error>
#include <type_traits>

namespace dr {
namespace yaskawa {

/// Struct holding details on an error.
struct ErrorDetails {
	boost::system::error_code code;
	std::string message;

	static ErrorDetails empty;
};

/// Get a reference to the Yaskawa error category.
impl::YaskawaCategory const & yaskawa_category();

/// Protocol error code constants.
namespace errc {
	enum errc_t {
		malformed_response  = 0x01,
		command_failed      = 0x02,
	};

	inline impl::YaskawaError make_error_code(errc_t code)      { return {code}; }
	inline impl::YaskawaError make_error_condition(errc_t code) { return {code}; }
}

/// Enum type for protocol error codes.
using errc_t = errc::errc_t;

}}

namespace std {
	template<> class is_error_code_enum     <dr::yaskawa::errc_t> : std::true_type {};
	template<> class is_error_condition_enum<dr::yaskawa::errc_t> : std::true_type {};
}

namespace boost {
namespace system {
	template<> struct is_error_code_enum     <dr::yaskawa::errc_t> : std::true_type {};
	template<> struct is_error_condition_enum<dr::yaskawa::errc_t> : std::true_type {};
}}
