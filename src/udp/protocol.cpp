#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/commands.hpp"

#include "encode.hpp"
#include "decode.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

void  ByteVariable::encode(std::vector<std::uint8_t> & out, std::uint8_t value) { writeLittleEndian<std::uint8_t>(out, value); }
void Int16Variable::encode(std::vector<std::uint8_t> & out, std::int16_t value) { writeLittleEndian<std::int16_t>(out, value); }
void Int32Variable::encode(std::vector<std::uint8_t> & out, std::int32_t value) { writeLittleEndian<std::int32_t>(out, value); }

ErrorOr<std::uint8_t>  ByteVariable::decode(string_view message) { return decodeIntegral< ByteVariable>(message, "byte"); }
ErrorOr<std::int16_t> Int16Variable::decode(string_view message) { return decodeIntegral<Int16Variable>(message, "int16"); }
ErrorOr<std::int32_t> Int32Variable::decode(string_view message) { return decodeIntegral<Int32Variable>(message, "int32"); }

void Float32Variable::encode(std::vector<std::uint8_t> & out, float value) {
	writeLittleEndian<std::uint32_t>(out, reinterpret_cast<std::uint32_t &>(value));
}

ErrorOr<float> Float32Variable::decode(string_view message) {
	if (message.size() != encoded_size) return unexpectedValue("float size", message.size(), encoded_size);
	std::int32_t parsed = readLittleEndian<std::int32_t>(message);
	return reinterpret_cast<float &>(parsed);
}

void PositionVariable::encode(std::vector<std::uint8_t> & out, Position const & position) {
	if (position.isPulse()) encodePulsePosition(out, position.pulse());
	else encodeCartesianPosition(out, position.cartesian());
}

/// Decode a position variable.
ErrorOr<Position> PositionVariable::decode(string_view data) {
	if (data.size() != 13 * 4) return unexpectedValue("message size", data.size(), 13 * 4);
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
	return Position{CartesianPosition{ {{
		readLittleEndian<std::int32_t>(data) / 1000.0,
		readLittleEndian<std::int32_t>(data) / 1000.0,
		readLittleEndian<std::int32_t>(data) / 1000.0,
		readLittleEndian<std::int32_t>(data) / 10000.0,
		readLittleEndian<std::int32_t>(data) / 10000.0,
		readLittleEndian<std::int32_t>(data) / 10000.0,
	}}, *frame, PoseConfiguration(configuration), int(tool)}};
}

}}}
