#include "types.hpp"

namespace dr {
namespace yaskawa {

std::string toString(CoordinateSystem system) {
	switch (system) {
		case CoordinateSystem::base: return "base";
		case CoordinateSystem::robot: return "robot";
		case CoordinateSystem::user1: return "user1";
		case CoordinateSystem::user2: return "user2";
		case CoordinateSystem::user3: return "user3";
		case CoordinateSystem::user4: return "user4";
		case CoordinateSystem::user5: return "user5";
		case CoordinateSystem::user6: return "user6";
		case CoordinateSystem::user7: return "user7";
		case CoordinateSystem::user8: return "user8";
		case CoordinateSystem::user9: return "user9";
		case CoordinateSystem::user10: return "user10";
		case CoordinateSystem::user11: return "user11";
		case CoordinateSystem::user12: return "user12";
		case CoordinateSystem::user13: return "user13";
		case CoordinateSystem::user14: return "user14";
		case CoordinateSystem::user15: return "user15";
		case CoordinateSystem::user16: return "user16";
		case CoordinateSystem::tool: return "tool";
		case CoordinateSystem::master: return "master";
	}
	throw std::logic_error{"invalid coordinate system: " + std::to_string(int(system))};
}

Result<CoordinateSystem> toCoordinateSystem(std::string const & string) {
	if (string == "base") return CoordinateSystem::base;
	if (string == "robot") return CoordinateSystem::robot;
	if (string == "user1") return CoordinateSystem::user1;
	if (string == "user2") return CoordinateSystem::user2;
	if (string == "user3") return CoordinateSystem::user3;
	if (string == "user4") return CoordinateSystem::user4;
	if (string == "user5") return CoordinateSystem::user5;
	if (string == "user6") return CoordinateSystem::user6;
	if (string == "user7") return CoordinateSystem::user7;
	if (string == "user8") return CoordinateSystem::user8;
	if (string == "user9") return CoordinateSystem::user9;
	if (string == "user10") return CoordinateSystem::user10;
	if (string == "user11") return CoordinateSystem::user11;
	if (string == "user12") return CoordinateSystem::user12;
	if (string == "user13") return CoordinateSystem::user13;
	if (string == "user14") return CoordinateSystem::user14;
	if (string == "user15") return CoordinateSystem::user15;
	if (string == "user16") return CoordinateSystem::user16;
	if (string == "tool") return CoordinateSystem::tool;
	if (string == "master") return CoordinateSystem::master;

	return Error{std::errc::invalid_argument, "invalid coordinate system: " + string};
}

std::ostream & operator<<(std::ostream & stream, CoordinateSystem const & frame) {
	if (isUserCoordinateSystem(frame)) {
		return stream << "user_frame_" << (userCoordinateNumber(frame));
	}
	switch (frame) {
		case CoordinateSystem::base:   return stream << "base";
		case CoordinateSystem::robot:  return stream << "robot";
		case CoordinateSystem::tool:   return stream << "tool";
		case CoordinateSystem::master: return stream << "master";
		default:                       return stream << "unknown_" << int(frame);
	}
}
std::ostream & operator<<(std::ostream & stream, PoseConfiguration const & configuration) {
	return stream
		<< (configuration.back()     ? "back,"   : "front,")
		<< (configuration.lowerArm() ? "lower,"  : "upper,")
		<< (configuration.highR()    ? "R>=180," : "R<180,")
		<< (configuration.highT()    ? "T>=180," : "T<180,")
		<< (configuration.highS()    ? "S>=180"  : "S<180")
	;
}

std::ostream & operator<<(std::ostream & stream, PulsePosition const & position) {
	stream << "PulsePosition{tool: " << position.tool() << ", joints: [";
	bool first = true;
	for (int pulse : position.joints()) {
		if (!first) {
			stream << ", ";
		}
		stream << pulse;
		first = false;
	}
	stream << "]}";
	return stream;
}

std::ostream & operator<<(std::ostream & stream, CartesianPosition const & position) {
	stream << "CartesianPosition{frame: " << position.frame() << ", tool: " << position.tool() << ", pose: [";
	bool first = true;
	for (double pulse : position) {
		if (!first) {
			stream << ", ";
		}
		stream << pulse;
		first = false;
	}
	stream << "], configuration: " << position.configuration() << "}";
	return stream;
}

std::ostream & operator<<(std::ostream & stream, Position const & position) {
	if (position.isPulse()) return stream << position.pulse();
	return stream << position.cartesian();
}


}}
