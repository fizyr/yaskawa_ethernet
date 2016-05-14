#pragma once

namespace dr {
namespace yaskawa {

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
