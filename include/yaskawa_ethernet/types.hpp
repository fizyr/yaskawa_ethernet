#pragma once
#include "array_view.hpp"

#include <boost/variant.hpp>

#include <array>
#include <bitset>
#include <ostream>

namespace dr {
namespace yaskawa {

struct Status {
	bool step;
	bool one_cycle;
	bool continuous;
	bool running;
	bool speed_limited;
	bool teach;
	bool play;
	bool remote;
	bool teach_pendant_hold;
	bool external_hold;
	bool command_hold;
	bool alarm;
	bool error;
	bool servo_on;
};

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

enum class CoordinateSystemType {
	robot_pulse     = 0,
	base_pulse      = 1,
	station_pulse   = 3,
	robot_cartesian = 4,
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

enum class SpeedType {
	joint,       // 0.01% of max speed
	translation, // 0.1 mm/s
	rotation     // 0.1 degrees/s
};

struct Speed {
	SpeedType type;
	std::uint32_t value;
};

enum class MoveFrame {
	base  = 16,
	robot = 17,
	user  = 18,
	tool  = 19,
};

/// Check if a coordinate system is a user coordinate system.
constexpr bool isUserCoordinateSystem(CoordinateSystem system) {
	return system >= CoordinateSystem::user1 && system <= CoordinateSystem::user16;
}

/// Get the one-based index of a user coordinate system.
/**
 * If a coordinate system is not a user coordinate system, this function returns 0.
 */
constexpr int userCoordinateNumber(CoordinateSystem system) {
	if (!isUserCoordinateSystem(system)) return 0;
	return int(system) - int(CoordinateSystem::user1) + 1;
}

/// Get a user coordinate system from a user coordinate system number (starting at 1).
constexpr CoordinateSystem userCoordinateSystem(int number) {
	return CoordinateSystem(int(CoordinateSystem::user1) + number - 1);
}

class PoseConfiguration : public std::bitset<6> {
public:
	PoseConfiguration() = default;
	PoseConfiguration(std::uint8_t type) noexcept : std::bitset<6>(type) {}
	PoseConfiguration(bool no_flip, bool lower_arm, bool back, bool high_r, bool high_t, bool high_s) :
		std::bitset<6>{no_flip * 0x01u | lower_arm * 0x02u | back * 0x04u |  high_r * 0x08u | high_t * 0x10u | high_s * 0x20u} {}

	bool      noFlip()   const noexcept { return (*this)[0]; }
	reference noFlip()         noexcept { return (*this)[0]; }
	bool      lowerArm() const noexcept { return (*this)[1]; }
	reference lowerArm()       noexcept { return (*this)[1]; }
	bool      back()     const noexcept { return (*this)[2]; }
	reference back()           noexcept { return (*this)[2]; }
	bool      highR()    const noexcept { return (*this)[3]; }
	reference highR()          noexcept { return (*this)[3]; }
	bool      highT()    const noexcept { return (*this)[4]; }
	reference highT()          noexcept { return (*this)[4]; }
	bool      highS()    const noexcept { return (*this)[5]; }
	reference highS()          noexcept { return (*this)[5]; }

	operator std::uint8_t () const noexcept { return to_ulong(); }
};

class PulsePosition {
private:
	std::array<int, 8> joints_;
	unsigned int size_;
	int tool_;

public:
	explicit PulsePosition(unsigned int size, int tool = 0) noexcept : size_{size}, tool_(tool) {}

	PulsePosition(std::array<int, 8> const & array, int tool = 0) noexcept : size_(array.size()), tool_(tool) {
		std::copy(array.begin(), array.end(), joints_.begin());
	}
	PulsePosition(std::array<int, 7> const & array, int tool = 0) noexcept : size_(array.size()), tool_(tool) {
		std::copy(array.begin(), array.end(), joints_.begin());
	}
	PulsePosition(std::array<int, 6> const & array, int tool = 0) noexcept : size_(array.size()), tool_(tool) {
		std::copy(array.begin(), array.end(), joints_.begin());
	}

	array_view<int      > joints()       noexcept { return {joints_.data(), size_}; }
	array_view<int const> joints() const noexcept { return {joints_.data(), size_}; }

	int & tool()       noexcept { return tool_; }
	int   tool() const noexcept { return tool_; }

	bool operator==(PulsePosition const & other) const {
		return size_ == other.size_
			&& tool_ == other.tool_
			&& joints_ == other.joints_;
	}

	bool operator!=(PulsePosition const & other) const {
		return !(*this == other);
	}
};

class CartesianPosition : public std::array<double, 6> {
private:
	CoordinateSystem frame_;
	PoseConfiguration type_;
	int tool_;

public:
	CartesianPosition() = default;

	CartesianPosition(
		std::array<double, 6> const & data,
		CoordinateSystem frame = CoordinateSystem::base,
		PoseConfiguration type = {},
		int tool = 0
	) :
		std::array<double, 6>(data),
		frame_{frame},
		type_{type},
		tool_(tool) {}

	CartesianPosition(
		double x, double y, double z,
		double rx, double ry, double rz,
		CoordinateSystem frame = CoordinateSystem::base,
		PoseConfiguration type = {},
		int tool = 0
	) : CartesianPosition{{{x, y, z, rx, ry, rz}}, frame, type, tool} {}

	CoordinateSystem   frame() const { return frame_; }
	CoordinateSystem & frame()       { return frame_; }

	PoseConfiguration   configuration() const { return type_; }
	PoseConfiguration & configuration()       { return type_; }

	int   tool() const { return tool_; }
	int & tool()       { return tool_; }

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

	bool operator==(CartesianPosition const & other) const {
		return frame_ == other.frame_
			&& type_ == other.type_
			&& tool_ == other.tool_
			&& static_cast<std::array<double, 6> const &>(*this) == other;
	}

	bool operator!=(CartesianPosition const & other) const {
		return !(*this == other);
	}
};

class Position {
	using Variant =  boost::variant<PulsePosition, CartesianPosition>;
	Variant data_;

public:
	Position() : data_(PulsePosition{0}) {}
	Position(PulsePosition      const & position) : data_(position) {}
	Position(CartesianPosition  const & position) : data_(position) {}

	PositionType type() const { return PositionType(data_.which());       }
	bool isPulse()      const { return type() == PositionType::pulse;     }
	bool isCartesian()  const { return type() == PositionType::cartesian; }

	PulsePosition const & pulse() const { return boost::get<PulsePosition const &>(data_); }
	PulsePosition       & pulse()       { return boost::get<PulsePosition       &>(data_); }

	CartesianPosition const & cartesian() const { return boost::get<CartesianPosition const &>(data_); }
	CartesianPosition       & cartesian()       { return boost::get<CartesianPosition       &>(data_); }

	bool operator==(Position const & other) const { return data_ == other.data_; }
	bool operator!=(Position const & other) const { return !(*this == other); }
};

std::ostream & operator<<(std::ostream & stream, CoordinateSystem const & frame);
std::ostream & operator<<(std::ostream & stream, PoseConfiguration const & configuration);
std::ostream & operator<<(std::ostream & stream, PulsePosition const & position);
std::ostream & operator<<(std::ostream & stream, PulsePosition const & position);
std::ostream & operator<<(std::ostream & stream, CartesianPosition const & position);
std::ostream & operator<<(std::ostream & stream, Position const & position);

}}
