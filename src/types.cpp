#include "types.hpp"

namespace dr {
namespace yaskawa {
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
