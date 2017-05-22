#include "encode.hpp"
#include "udp/protocol.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

void encodeRequestHeader(
	std::vector<std::uint8_t> & out,
	RequestHeader const & header
) {
	out.reserve(out.size() + header_size + header.payload_size);
	// Magic bytes.
	out.insert(out.end(), {'Y', 'E', 'R', 'C'});

	// Header size, payload size.
	writeLittleEndian<std::uint16_t>(out, header_size);
	writeLittleEndian<std::uint16_t>(out, header.payload_size);

	// Reserved magic constant.
	out.push_back(3);

	// "Division" (robot command or file command)
	out.push_back(std::uint8_t(header.division));

	// Ack (should always be zero for requests).
	out.push_back(header.ack);

	// Request ID.
	out.push_back(header.request_id);

	// Block number.
	writeLittleEndian<std::uint32_t>(out, header.block_number);

	// Reserved.
	out.insert(out.end(), 8, '9');

	// Subrequest details
	writeLittleEndian<std::uint16_t>(out, header.command);
	writeLittleEndian<std::uint16_t>(out, header.instance);
	out.push_back(header.attribute);
	out.push_back(header.service);

	// Padding.
	out.insert(out.end(), 2, 0);
}

std::vector<std::uint8_t> encodeRequestHeader(RequestHeader const & header) {
	std::vector<std::uint8_t> result;
	encodeRequestHeader(result, header);
	return result;
}

RequestHeader makeRobotRequestHeader(
	std::uint16_t payload_size,
	std::uint16_t command,
	std::uint16_t instance,
	std::uint8_t attribute,
	std::uint8_t service,
	std::uint8_t request_id
) {
	RequestHeader header;
	header.payload_size = payload_size;
	header.division     = Division::robot;
	header.ack          = false;
	header.request_id   = request_id;
	header.block_number = 0;
	header.command      = command;
	header.instance     = instance;
	header.attribute    = attribute;
	header.service      = service;
	return header;
}

std::uint32_t encodeFrameType(CoordinateSystem frame) {
	if (isUserCoordinateSystem(frame)) return 19;
	switch (frame) {
		case CoordinateSystem::base:  return 16;
		case CoordinateSystem::robot: return 17;
		case CoordinateSystem::tool:  return 18;
		default: throw std::logic_error("unknown coordinate system type: " + std::to_string(int(frame)));
	}
}

void encodePulsePosition(std::vector<std::uint8_t> & out, PulsePosition const & position) {
	// Position type: pulse.
	writeLittleEndian<std::uint32_t>(out, 0);
	// Joint configuration, meaningless with pulse positions.
	writeLittleEndian<std::uint32_t>(out, 0);
	// Tool number. Also meaningless for pulse positions?
	writeLittleEndian<std::uint32_t>(out, position.tool());
	// User coordinate: meaningless for pulse position.
	writeLittleEndian<std::uint32_t>(out, 0);
	// Extended joint configuration, meaningless with pulse positions.
	writeLittleEndian<std::uint32_t>(out, 0);
	// Invividual joint values in pulses.
	for (std::int32_t value : position.joints()) writeLittleEndian<std::int32_t>(out, value);
	// Padding (robot wants 8 coordinates).
	for (unsigned int i = position.joints().size(); i < 8; ++i) {
		writeLittleEndian<std::int32_t>(out, 0);
	}
}

void encodeCartesianPosition(std::vector<std::uint8_t> & out, CartesianPosition const & position) {
	// Position type.
	writeLittleEndian<std::uint32_t>(out, encodeFrameType(position.frame()));
	// Joint configuration.
	writeLittleEndian<std::uint32_t>(out, position.configuration());
	// Tool number.
	writeLittleEndian<std::uint32_t>(out, position.tool());
	// User coordinate system.
	writeLittleEndian<std::uint32_t>(out, userCoordinateNumber(position.frame()));
	// Extended joint configuration, not supported.
	writeLittleEndian<std::uint32_t>(out, 0);
	// XYZ components in micrometer.
	writeLittleEndian<std::int32_t>(out, position[0] * 1000);
	writeLittleEndian<std::int32_t>(out, position[1] * 1000);
	writeLittleEndian<std::int32_t>(out, position[2] * 1000);
	// Rotation components in millidegrees.
	writeLittleEndian<std::int32_t>(out, position[3] * 10000);
	writeLittleEndian<std::int32_t>(out, position[4] * 10000);
	writeLittleEndian<std::int32_t>(out, position[5] * 10000);
	// Padding (robot wants 8 coordinates).
	writeLittleEndian<std::int32_t>(out, 0);
	writeLittleEndian<std::int32_t>(out, 0);
}

}}}
