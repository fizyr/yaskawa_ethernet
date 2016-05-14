#pragma once
#include "error.hpp"

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/variant.hpp>

#include <string>
#include <utility>

namespace dr {
namespace yaskawa {

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

enum class VariableType {
	byte_type                  = 0,
	integer_type               = 1,
	double_type                = 2,
	real_type                  = 3,
	robot_axis_position_type   = 4,
	base_axis_position_type    = 5,
	station_axis_position_type = 6,
	string_type                = 7,
};

}}
