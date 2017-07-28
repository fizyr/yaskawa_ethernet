#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/commands.hpp"

#include "encode.hpp"
#include "decode.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

ErrorOr<Position> ReadCurrentRobotPosition::decode(string_view & data) {
	if (data.size() > 13 * 4) return DetailedError{std::errc::invalid_argument, "current robot position response larger than expected, expected at most 52 bytes, got " + std::to_string(data.size()) + " bytes"};
	std::string padded_data;
	padded_data.resize(13 * 4, '0');
	std::copy(data.begin(), data.end(), padded_data.begin());
	string_view padded_view = padded_data;
	ErrorOr<Position> result = PositionVariable::decode(padded_view);
	if (!result) return result.error();
	return result;
}

void  ByteVariable::encode(std::vector<std::uint8_t> & out, std::uint8_t value) { writeLittleEndian<std::uint8_t>(out, value); }
void Int16Variable::encode(std::vector<std::uint8_t> & out, std::int16_t value) { writeLittleEndian<std::int16_t>(out, value); }
void Int32Variable::encode(std::vector<std::uint8_t> & out, std::int32_t value) { writeLittleEndian<std::int32_t>(out, value); }

ErrorOr<std::uint8_t>  ByteVariable::decode(string_view & message) { return readLittleEndian<std::uint8_t>(message); }
ErrorOr<std::int16_t> Int16Variable::decode(string_view & message) { return readLittleEndian<std::int16_t>(message); }
ErrorOr<std::int32_t> Int32Variable::decode(string_view & message) { return readLittleEndian<std::int32_t>(message); }

void Float32Variable::encode(std::vector<std::uint8_t> & out, float value) {
	writeLittleEndian<std::uint32_t>(out, reinterpret_cast<std::uint32_t &>(value));
}

ErrorOr<float> Float32Variable::decode(string_view & message) {
	std::int32_t parsed = readLittleEndian<std::int32_t>(message);
	return reinterpret_cast<float &>(parsed);
}

void PositionVariable::encode(std::vector<std::uint8_t> & out, Position const & position) {
	if (position.isPulse()) encodePulsePosition(out, position.pulse());
	else encodeCartesianPosition(out, position.cartesian());
}

/// Decode a position variable.
ErrorOr<Position> PositionVariable::decode(string_view & data) {
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

void ReadFileList::encode(std::vector<std::uint8_t> & message, string_view type) {
	std::copy(type.begin(), type.end(), std::back_inserter(message));
}

ErrorOr<std::vector<std::string>> ReadFileList::decode(string_view & data) {
	if (data.size() == 0) return std::vector<std::string>{};
	if (data.size() == 1) return DetailedError{std::errc::invalid_argument, "file list consist of exactly one byte"};
	auto line_start = data.begin();
	auto finger     = line_start;
	std::vector<std::string> result;
	while (finger < data.end() - 1) {
		if (finger[0] == '\r' && finger[1] == '\n') {
			result.emplace_back(line_start, finger);
			line_start = finger + 2;
		}
	}
	return result;
}

void ReadFile::encode(std::vector<std::uint8_t> & message, string_view name) {
	std::copy(name.begin(), name.end(), std::back_inserter(message));
}

ErrorOr<void> ReadFile::decode(string_view &) {
	return in_place_valid;
}

void WriteFile::encode(std::vector<std::uint8_t> & message, string_view name) {
	std::copy(name.begin(), name.end(), std::back_inserter(message));
}

ErrorOr<void> WriteFile::decode(string_view &) {
	return in_place_valid;
}

void DeleteFile::encode(std::vector<std::uint8_t> & message, string_view name) {
	std::copy(name.begin(), name.end(), std::back_inserter(message));
}

ErrorOr<void> DeleteFile::decode(string_view &) {
	return in_place_valid;
}

void FileData::encode(std::vector<std::uint8_t> & message, string_view data) {
	std::copy(data.begin(), data.end(), std::back_inserter(message));
}

ErrorOr<std::string> FileData::decode(string_view & data) {
	std::string result{data.begin(), data.end()};
	data.remove_prefix(data.size());
	return result;
}

}}}
