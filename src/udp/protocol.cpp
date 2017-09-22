#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/commands.hpp"

#include "encode.hpp"
#include "decode.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

ErrorOr<Status> StatusInformation::decode(string_view & data) {
	Status result;
	result.step          = data[0] & (1 << 0);
	result.one_cycle     = data[0] & (1 << 1);
	result.continuous    = data[0] & (1 << 2);
	result.running       = data[0] & (1 << 3);
	result.speed_limited = data[0] & (1 << 4);
	result.teach         = data[0] & (1 << 5);
	result.play          = data[0] & (1 << 6);
	result.remote        = data[0] & (1 << 7);

	result.teach_pendant_hold = data[4] & (1 << 1);
	result.external_hold      = data[4] & (1 << 2);
	result.command_hold       = data[4] & (1 << 3);
	result.alarm              = data[4] & (1 << 4);
	result.error              = data[4] & (1 << 5);
	result.servo_on           = data[4] & (1 << 6);
	return result;
}

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

int systemToMoveLSystem(CoordinateSystem system) {
	if (system == CoordinateSystem::base) return 16;
	if (system == CoordinateSystem::robot) return 17;
	if (isUserCoordinateSystem(system)) return 18;
	if (system == CoordinateSystem::tool) return 19;
	throw std::runtime_error{"invalid coordinate system for MoveL: " + std::to_string(int(system))};
}

void MoveL::encode(std::vector<std::uint8_t> & out, CartesianPosition const & target, int control_group, Speed speed) {
	writeLittleEndian<std::uint32_t>(out, control_group + 1);
	writeLittleEndian<std::uint32_t>(out, 0); // Station control group.
	writeLittleEndian<std::uint32_t>(out, std::int32_t(speed.type));
	writeLittleEndian<std::uint32_t>(out, std::int32_t(speed.value));
	writeLittleEndian<std::uint32_t>(out, systemToMoveLSystem(target.frame()));

	// Translation coordinates in 1e-6 meters.
	writeLittleEndian<std::int32_t>(out, target[0] * 1000);
	writeLittleEndian<std::int32_t>(out, target[1] * 1000);
	writeLittleEndian<std::int32_t>(out, target[2] * 1000);
	// Rotation components in 1e-4 degrees.
	writeLittleEndian<std::int32_t>(out, target[3] * 10000);
	writeLittleEndian<std::int32_t>(out, target[4] * 10000);
	writeLittleEndian<std::int32_t>(out, target[5] * 10000);

	writeLittleEndian<std::uint32_t>(out, 0); // reserved
	writeLittleEndian<std::uint32_t>(out, 0); // reserved
	writeLittleEndian<std::uint32_t>(out, target.configuration());
	writeLittleEndian<std::uint32_t>(out, 0); // extended type
	writeLittleEndian<std::uint32_t>(out, target.tool());
	writeLittleEndian<std::uint32_t>(out, userCoordinateNumber(target.frame()));

	// unsupported base and station axes.
	for (int i = 18; i <= 26; ++i) writeLittleEndian<std::uint32_t>(out, 0);

}

}}}
