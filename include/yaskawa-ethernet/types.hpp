#pragma once
#include <array>
#include <boost/variant.hpp>
#include <bitset>
#include "array_view.hpp"

namespace dr {
namespace yaskawa {

enum class VariableType {
	byte_type             = 0,
	integer_type          = 1,
	double_type           = 2,
	real_type             = 3,
	robot_position_type   = 4,
	base_position_type    = 5,
	station_position_type = 6,
	string_type           = 7,
};

enum class PositionType {
	pulse     = 0,
	cartesian = 1,
};

enum class CoordinateSystem {
	base   = 0,
	robot  = 1,
	user1  = 2 ,
	user2  = 3 ,
	user3  = 4 ,
	user4  = 5 ,
	user5  = 6 ,
	user6  = 7 ,
	user7  = 8 ,
	user8  = 9 ,
	user9  = 10,
	user10 = 11,
	user11 = 12,
	user12 = 13,
	user13 = 14,
	user14 = 15,
	user15 = 16,
	user16 = 17,
	tool   = 18,
	master = 19,
};

class CartesianPoseType : public std::bitset<5> {
public:
	CartesianPoseType() = default;
	CartesianPoseType(std::uint8_t type) noexcept : std::bitset<5>(type) {}
	CartesianPoseType(bool flip, bool lower_arm, bool high_r, bool high_t, bool high_s) :
		std::bitset<5>{flip * 0x01u | lower_arm * 0x02u |  high_r * 0x04u | high_t * 0x08u | high_s * 0x10u} {}

	bool      flip()     const noexcept { return (*this)[0]; }
	reference flip()           noexcept { return (*this)[0]; }
	bool      lowerArm() const noexcept { return (*this)[1]; }
	reference lowerArm()       noexcept { return (*this)[1]; }
	bool      highR()    const noexcept { return (*this)[2]; }
	reference highR()          noexcept { return (*this)[2]; }
	bool      highT()    const noexcept { return (*this)[3]; }
	reference highT()          noexcept { return (*this)[3]; }
	bool      highS()    const noexcept { return (*this)[4]; }
	reference highS()          noexcept { return (*this)[4]; }

	operator std::uint8_t () const noexcept { return to_ulong(); }
};

class PulsePosition {
private:
	std::array<int, 7> joints_;
	bool axis7_;
	int tool_;

public:
	PulsePosition(bool axis7, int tool = 0) noexcept : axis7_{axis7}, tool_(tool) {}
	PulsePosition(std::array<int, 7> const & array, int tool = 0) noexcept : joints_{array}, axis7_{true}, tool_(tool) {}
	PulsePosition(std::array<int, 6> const & array, int tool = 0) noexcept :
		joints_{{array[0], array[1], array[2], array[3], array[4], array[5], 0}},
		axis7_{false},
		tool_(tool) {}

	array_view<int      > joints()       noexcept { return {joints_.data(), axis7_ ? 7u : 6u}; }
	array_view<int const> joints() const noexcept { return {joints_.data(), axis7_ ? 7u : 6u}; }

	int & tool()       noexcept { return tool_; }
	int   tool() const noexcept { return tool_; }
};

struct CartesianPosition : std::array<double, 6> {
	CoordinateSystem system;
	CartesianPoseType type;
	int tool;

	CartesianPosition() = default;

	CartesianPosition(
		std::array<double, 6> const & data,
		CoordinateSystem system = CoordinateSystem::base,
		CartesianPoseType type = {},
		int tool = 0
	) :
		std::array<double, 6>(data),
		system{system},
		type{type},
		tool(tool) {}

	CartesianPosition(
		double x, double y, double z,
		double rx, double ry, double rz,
		CoordinateSystem system = CoordinateSystem::base,
		CartesianPoseType type = {},
		int tool = 0
	) : CartesianPosition{{{x, y, z, rx, ry, rz}}, system, type, tool} {}

	double    x() const { return (*this)[0]; }
	double &  x()       { return (*this)[0]; }
	double    y() const { return (*this)[1]; }
	double &  y()       { return (*this)[1]; }
	double    z() const { return (*this)[2]; }
	double &  z()       { return (*this)[2]; }
	double   rx() const { return (*this)[3]; }
	double & rx()       { return (*this)[3]; }
	double   ry() const { return (*this)[4]; }
	double & ry()       { return (*this)[4]; }
	double   rz() const { return (*this)[5]; }
	double & rz()       { return (*this)[5]; }
};

class Position {
	using Variant =  boost::variant<PulsePosition, CartesianPosition>;
	Variant data_;

public:
	Position(PulsePosition const & position) : data_(position) {}
	Position(PulsePosition      && position) : data_(std::move(position)) {}
	Position(CartesianPosition  const & position) : data_(position) {}
	Position(CartesianPosition       && position) : data_(std::move(position)) {}

	PositionType type() const { return PositionType(data_.which()); }
	bool isPulse()     const { return type() == PositionType::pulse;     }
	bool isCartesian() const { return type() == PositionType::cartesian; }

	PulsePosition const & pulse() const { return boost::get<PulsePosition const &>(data_); }
	PulsePosition       & pulse()       { return boost::get<PulsePosition &>(data_);       }

	CartesianPosition const & cartesian() const { return boost::get<CartesianPosition const &>(data_); }
	CartesianPosition       & cartesian()       { return boost::get<CartesianPosition &>(data_);       }


};

}}
