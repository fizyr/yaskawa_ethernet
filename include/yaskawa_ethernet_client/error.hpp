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

/// Struct holding details on an error.
class DetailedError : public boost::system::error_code {
	std::string details_;

public:
	using boost::system::error_code::error_code;
	DetailedError() = default;

	DetailedError(boost::system::error_code const & error, std::string const & details) : boost::system::error_code(error), details_(details) {}
	DetailedError(boost::system::error_code const & error, std::string      && details) : boost::system::error_code(error), details_(std::move(details)) {}

	std::string detailed_message() const {
		if (details_.empty()) return message();
		return boost::system::error_code::message() + ": " + details_;
	}

	std::string const & details() const noexcept { return details_; }

	void clear() {
		boost::system::error_code::clear();
		details_.clear();
	}

	boost::system::system_error to_system_error() const {
		if (details_.empty()) return boost::system::system_error(*this);
		return boost::system::system_error(*this, details_);
	}

	static DetailedError empty;
};

template<typename T>
class ErrorOr {
	boost::variant<DetailedError, T> data_;

public:
	ErrorOr(DetailedError const & details) : data_(details) {};
	ErrorOr(DetailedError      && details) : data_(std::move(details)) {};
	ErrorOr(boost::system::error_code const & error) : ErrorOr(DetailedError{error, ""}) {}
	ErrorOr(boost::system::error_code const & error, std::string const & message) : ErrorOr(DetailedError{error, message}) {}
	ErrorOr(boost::system::error_code const & error, std::string      && message) : ErrorOr(DetailedError{error, std::move(message)}) {}

	ErrorOr(T const & value) : data_{value} {}
	ErrorOr(T      && value) : data_{std::move(value)} {}

	template<typename T2, typename N = typename std::enable_if<std::is_convertible<T, T2>::value>::type>
	operator ErrorOr<T2> () {
		if (!valid()) return ErrorOr<T2>{error()};
		return ErrorOr<T2>{T2(get())};
	}

	bool valid() const {
		return data_.which() == 1;
	}

	DetailedError const & error() const {
		if (valid()) return DetailedError::empty;
		return boost::get<DetailedError>(data_);
	}

	std::string errorMessage() const {
		return error().detailed_message();
	}


	T const & get() const {
		if (!valid()) throw error().to_system_error();
		return boost::get<T>(data_);
	}

	T & get() {
		if (!valid()) throw error().to_system_error();
		return boost::get<T>(data_);
	}
};

template<>
class ErrorOr<void> {
	DetailedError data_;

public:
	ErrorOr(DetailedError const & details) : data_(details) {};
	ErrorOr(DetailedError      && details) : data_(std::move(details)) {};
	ErrorOr(boost::system::error_code const & error) : ErrorOr(DetailedError{error, ""}) {}
	ErrorOr(boost::system::error_code const & error, std::string const & message) : ErrorOr(DetailedError{error, message}) {}
	ErrorOr(boost::system::error_code const & error, std::string      && message) : ErrorOr(DetailedError{error, std::move(message)}) {}

	ErrorOr() {}

	bool valid() const {
		return !data_;
	}

	DetailedError const & error() const {
		if (valid()) return DetailedError::empty;
		return data_;
	}

	std::string errorMessage() const {
		return error().detailed_message();
	}

	void get() const {
		if (!valid()) throw error().to_system_error();
	}
};

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
