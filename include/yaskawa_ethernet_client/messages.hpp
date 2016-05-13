#pragma once
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/variant.hpp>

#include <string>
#include <utility>

namespace dr {
namespace yaskawa {

template<typename T>
class ErrorOr {
	boost::variant<boost::system::error_code, T> data_;

public:
	ErrorOr(boost::system::error_code const & error) : data_{error} {}

	ErrorOr(T const & value) : data_{value} {}
	ErrorOr(T      && value) : data_{std::move(value)} {}

	bool valid() const {
		return data_.which() == 1;
	}

	boost::system::error_code error() const {
		if (valid()) return boost::system::error_code{};
		return boost::get<boost::system::error_code>(data_);
	}

	T const & get() const {
		boost::system::error_code error = this->error();
		if (error) throw boost::system::system_error(error);
		return boost::get<T>(data_);
	}

	T & get() {
		boost::system::error_code error = this->error();
		if (error) throw boost::system::system_error(error);
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

struct Response {
	bool success;
	std::string message;

	explicit operator bool() const { return success; }
};

}}
