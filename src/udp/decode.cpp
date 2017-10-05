#include "decode.hpp"
#include "udp/protocol.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

ErrorOr<ResponseHeader> decodeResponseHeader(string_view & data) {
	string_view original = data;
	ResponseHeader result;

	// Check that the message is large enough to hold the header.
	if (auto error = expectSizeMin("response", data.size(), header_size)) return error;

	// Check the magic bytes.
	if (data.substr(0, 4) != "YERC") return malformedResponse("response does not start with magic bytes `YERC'");
	data.remove_prefix(4);

	// Check the header size.
	std::uint16_t parsed_header_size = readLittleEndian<std::uint16_t>(data);
	if (auto error = expectValue("header size", parsed_header_size, header_size)) return error;

	// Get payload size and make sure the message is complete.
	result.payload_size = readLittleEndian<std::uint16_t>(data);
	if (auto error = expectValueMax("payload size", result.payload_size, max_payload_size)) return error;

	data.remove_prefix(1);
	result.division = Division(readLittleEndian<std::uint8_t>(data));

	// Make sure the ack value is correct.
	std::uint8_t ack = readLittleEndian<std::uint8_t>(data);
	if (auto error = expectValue("ACK value", ack, 1)) return error;
	result.ack = true;

	// Parse request ID and block number.
	result.request_id   = readLittleEndian<std::uint8_t>(data);
	result.block_number = readLittleEndian<std::uint32_t>(data);

	// Reserved 8 bytes.
	data.remove_prefix(8);

	// Parse service and status field.
	result.service = readLittleEndian<std::uint8_t>(data);
	result.status  = readLittleEndian<std::uint8_t>(data);

	// Ignore added status size, just treat it as two byte value.
	data.remove_prefix(2);
	result.extra_status = readLittleEndian<std::uint16_t>(data);

	// Padding.
	data.remove_prefix(2);

	if (original.size() != header_size + result.payload_size) return malformedResponse(
		"request " + std::to_string(int(result.request_id)) + ": "
		"number of received bytes (" + std::to_string(original.size()) + ") "
		"does not match the message size according to the header "
		"(" + std::to_string(header_size + result.payload_size) + ")"
	);

	return result;
}

template<> ErrorOr<std::uint8_t> decode<std::uint8_t>(string_view & data) {
	return readLittleEndian<std::uint8_t>(data);
}
template<> ErrorOr<std::int16_t> decode<std::int16_t>(string_view & data) {
	return readLittleEndian<std::int16_t>(data);
}

template<> ErrorOr<std::int32_t> decode<std::int32_t>(string_view & data) {
	return readLittleEndian<std::int32_t>(data);
}

template<> ErrorOr<float> decode<float>(string_view & data) {
	std::uint32_t integral = readLittleEndian<std::uint32_t>(data);
	return reinterpret_cast<float const &>(integral);
}

namespace {
	ErrorOr<CoordinateSystem> decodeCartesianFrame(int type, int user_frame) {
		switch (type) {
			case 16: return CoordinateSystem::base;
			case 17: return CoordinateSystem::robot;
			case 18: return CoordinateSystem::tool;
			case 19:
				if (auto error = expectValueMax("user frame", user_frame, 15)) return error;
				return userCoordinateSystem(user_frame);
		}
		return malformedResponse("unknown position type (" + std::to_string(type) + "), expected 16, 17, 18 or 19");
	}
}

template<> ErrorOr<Position> decode<Position>(string_view & data) {
	std::uint32_t type                   = readLittleEndian<std::uint32_t>(data);
	std::uint8_t  configuration          = readLittleEndian<std::uint32_t>(data);
	std::uint32_t tool                   = readLittleEndian<std::uint32_t>(data);
	std::uint32_t user_frame             = readLittleEndian<std::uint32_t>(data);
	std::uint8_t  extended_configuration = readLittleEndian<std::uint32_t>(data);

	// Extended joint configuration is not supported.
	(void) extended_configuration;

	// Pulse position.
	if (type == 0) {
		PulsePosition result(8, tool);
		for (int i = 0; i < 8; ++i) result.joints()[i] = readLittleEndian<std::int32_t>(data);
		return Position{result};
	}

	ErrorOr<CoordinateSystem> frame = decodeCartesianFrame(type, user_frame);
	if (!frame) return frame.error();

	// Cartesian position.
	// Position data is in micrometers.
	// Rotation data is in 0.0001 degrees.
	Position result = CartesianPosition{ {{
		readLittleEndian<std::int32_t>(data) / 1e3,
		readLittleEndian<std::int32_t>(data) / 1e3,
		readLittleEndian<std::int32_t>(data) / 1e3,
		readLittleEndian<std::int32_t>(data) / 1e4,
		readLittleEndian<std::int32_t>(data) / 1e4,
		readLittleEndian<std::int32_t>(data) / 1e4,
	}}, *frame, PoseConfiguration(configuration), int(tool)};

	// Remove padding.
	data.remove_prefix(8);

	return result;
}

}}}
