#pragma once
#include "impl/error.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/variant.hpp>

#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

namespace dr {
namespace yaskawa {

/// Struct holding details on an error.
struct ErrorDetails {
	boost::system::error_code code;
	std::string message;

	static ErrorDetails empty;
};

template<typename T>
class ErrorOr {
	boost::variant<ErrorDetails, T> data_;

public:
	ErrorOr(ErrorDetails const & details) : data_(details) {};
	ErrorOr(ErrorDetails      && details) : data_(std::move(details)) {};
	ErrorOr(boost::system::error_code const & error) : ErrorOr(ErrorDetails{error, ""}) {}
	ErrorOr(boost::system::error_code const & error, std::string const & message) : ErrorOr(ErrorDetails{error, message}) {}
	ErrorOr(boost::system::error_code const & error, std::string      && message) : ErrorOr(ErrorDetails{error, std::move(message)}) {}

	ErrorOr(T const & value) : data_{value} {}
	ErrorOr(T      && value) : data_{std::move(value)} {}

	bool valid() const {
		return data_.which() == 1;
	}

	ErrorDetails const & errorDetails() const {
		if (valid()) return ErrorDetails::empty;
		return boost::get<ErrorDetails>(data_);
	}

	boost::system::error_code error() const {
		return errorDetails().code;
	}

	std::string const & errorMessage() const {
		return errorDetails().message;
	}

	T const & get() const {
		if (!valid()) throw boost::system::system_error(error(), errorMessage());
		return boost::get<T>(data_);
	}

	T & get() {
		if (!valid()) throw boost::system::system_error(error(), errorMessage());
		return boost::get<T>(data_);
	}
};

template<>
class ErrorOr<void> {
	ErrorDetails data_;

public:
	ErrorOr(ErrorDetails const & details) : data_(details) {};
	ErrorOr(ErrorDetails      && details) : data_(std::move(details)) {};
	ErrorOr(boost::system::error_code const & error) : ErrorOr(ErrorDetails{error, ""}) {}
	ErrorOr(boost::system::error_code const & error, std::string const & message) : ErrorOr(ErrorDetails{error, message}) {}
	ErrorOr(boost::system::error_code const & error, std::string      && message) : ErrorOr(ErrorDetails{error, std::move(message)}) {}

	ErrorOr() {}

	bool valid() const {
		return !data_.code;
	}

	ErrorDetails const & errorDetails() const {
		if (valid()) return ErrorDetails::empty;
		return data_;
	}

	boost::system::error_code error() const {
		return errorDetails().code;
	}

	std::string const & errorMessage() const {
		return errorDetails().message;
	}

	void get() const {
		if (!valid()) throw boost::system::system_error(error(), errorMessage());
	}
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
