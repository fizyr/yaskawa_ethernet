#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/command_traits.hpp"

#include "encode.hpp"
#include "decode.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

using namespace std::string_literals;

/// Encode a ReadStatus command.
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, ReadStatus const &) {
	int payload_size = 0;
	int instance = 1;
	int attribute = 0;
	encode(output, makeRobotRequestHeader(
		payload_size,
		commands::robot::read_status_information,
		instance,
		attribute,
		service::get_all,
		request_id
	));
}

/// Decode a ReadStatus response.
ErrorOr<Status> decode(ResponseHeader const &, string_view & data, ReadStatus const &) {
	if (auto error = checkSize("status data", data, 8)) return error;

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

/// Encode a ReadCurrentPosition command.
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, ReadCurrentPosition const & command) {
	constexpr int payload_size = 0;
	constexpr int attribute = 0;
	int instance = command.control_group;

	switch (command.coordinate_system) {
		case CoordinateSystemType::robot_pulse:     instance +=   1; break;
		case CoordinateSystemType::base_pulse:      instance +=  11; break;
		case CoordinateSystemType::station_pulse:   instance +=  21; break;
		case CoordinateSystemType::robot_cartesian: instance += 101; break;
	}

	encode(output, makeRobotRequestHeader(
		payload_size,
		commands::robot::read_robot_position,
		instance,
		attribute,
		service::get_all,
		request_id
	));
}

/// Decode a ReadCurrentPosition command.
ErrorOr<Position> decode(ResponseHeader const &, string_view & message, ReadCurrentPosition const &) {
	if (auto error = checkSizeMax("position data", message, 13 * 4)) return error;

	// Pad the data until it is 13 * 4 bytes.
	std::string padded_data;
	padded_data.resize(13 * 4, '\0');
	std::copy(message.begin(), message.end(), padded_data.begin());
	string_view padded_view = padded_data;
	return decode<Position>(padded_view);
}

namespace {
	/// Convert a coordinate system to a cartesian move command coordinate system type.
	int systemToMoveLSystem(CoordinateSystem system) {
		if (system == CoordinateSystem::base) return 16;
		if (system == CoordinateSystem::robot) return 17;
		if (isUserCoordinateSystem(system)) return 18;
		if (system == CoordinateSystem::tool) return 19;
		throw std::runtime_error{"invalid coordinate system for MoveL: " + std::to_string(int(system))};
	}
}

/// Encode a MoveL command.
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, MoveL const & command) {
	constexpr int payload_size = 26 * 4;
	constexpr int instance     = 2; // Absolute cartesian interpolated move.
	constexpr int attribute    = 1;

	encode(output, makeRobotRequestHeader(
		payload_size,
		commands::robot::read_robot_position,
		instance,
		attribute,
		service::get_all,
		request_id
	));

	writeLittleEndian<std::uint32_t>(output, command.control_group + 1);
	writeLittleEndian<std::uint32_t>(output, 0); // Station control group.
	writeLittleEndian<std::uint32_t>(output, std::int32_t(command.speed.type));
	writeLittleEndian<std::uint32_t>(output, std::int32_t(command.speed.value));
	writeLittleEndian<std::uint32_t>(output, systemToMoveLSystem(command.target.frame()));

	// Translation coordinates in 1e-6 meters.
	writeLittleEndian<std::int32_t>(output, command.target[0] * 1000);
	writeLittleEndian<std::int32_t>(output, command.target[1] * 1000);
	writeLittleEndian<std::int32_t>(output, command.target[2] * 1000);
	// Rotation components in 1e-4 degrees.
	writeLittleEndian<std::int32_t>(output, command.target[3] * 10000);
	writeLittleEndian<std::int32_t>(output, command.target[4] * 10000);
	writeLittleEndian<std::int32_t>(output, command.target[5] * 10000);

	writeLittleEndian<std::uint32_t>(output, 0); // reserved
	writeLittleEndian<std::uint32_t>(output, 0); // reserved
	writeLittleEndian<std::uint32_t>(output, command.target.configuration());
	writeLittleEndian<std::uint32_t>(output, 0); // extended type
	writeLittleEndian<std::uint32_t>(output, command.target.tool());
	writeLittleEndian<std::uint32_t>(output, userCoordinateNumber(command.target.frame()));

	// unsupported base and station axes.
	for (int i = 18; i <= 26; ++i) writeLittleEndian<std::uint32_t>(output, 0);
}

/// Decode a MoveL response.
ErrorOr<void> decode(ResponseHeader const &, string_view & data, MoveL const &) {
	if (auto error = checkSize("response data", data, 0)) return error;
	return in_place_valid;
}

namespace {
	/// Encode a ReadVar command.
	template<typename Command>
	void encodeReadVar(std::vector<std::uint8_t> & output, std::uint8_t request_id, Command const & command) {
		if (command.count == 1) {
			encode(output, makeRobotRequestHeader(0, single_command<Command>(), command.index, 0, service::get_all, request_id));
		} else {
			encode(output, makeRobotRequestHeader(4, multi_command<Command>(), command.index, 0, service::read_multiple, request_id));
			writeLittleEndian<std::uint32_t>(output, command.count);
		}
	}

	/// Decode a ReadVar response.
	template<typename Command>
	ErrorOr<typename Command::Response> decodeReadVar(string_view & message, Command const & command) {
		using Type = typename Command::Response::value_type;

		// Read a single value (data is exactly one element).
		if (command.count == 1) {
			if (auto error = checkSize( "response data", message, encoded_size<Command>())) return error;
			ErrorOr<Type> decoded = decode<Type>(message);
			if (!decoded) return decoded.error_unchecked();
			return std::vector<Type>{*decoded};
		}

		// Read multiple values (data starts with a 32 bit value count).
		if (auto error = checkSize( "response data", message, 4 + command.count * encoded_size<Command>())) return error;

		// Check if value count matches our request.
		std::uint32_t count = readLittleEndian<std::uint32_t>(message);
		if (auto error = checkValue("value count", count, command.count)) return error;

		// Decode and return values.
		std::vector<Type> result;
		result.reserve(command.count);
		for (std::size_t i = 0; i < command.count; ++i) {
			ErrorOr<Type> decoded = decode<Type>(message);
			if (!decoded) return decoded.error_unchecked();
			result.push_back(*decoded);
		}
		return result;
	}

	/// Encode a WriteVar command.
	template<typename Command>
	void encodeWriteVar(std::vector<std::uint8_t> & output, std::uint8_t request_id, Command const & command) {
		std::uint32_t data_size = command.values.size() * encoded_size<Command>();

		// Write a single value.
		if (command.values.size() == 1) {
			encode(output, makeRobotRequestHeader(data_size, single_command<Command>(), command.index, 0, service::set_all, request_id));
			encode(output, command.values[0]);

		// Write mutliple values.
		} else {
			encode(output, makeRobotRequestHeader(4 + data_size, multi_command<Command>(), command.index, 0, service::write_multiple, request_id));
			writeLittleEndian<std::uint32_t>(output, command.values.size());
			for (auto const & val : command.values) {
				encode(output, val);
			}
		}
	}

	/// Decode a WriteVar response.
	template<typename Command>
	ErrorOr<void> decodeWriteVar(string_view & data, Command const &) {
		if (auto error = checkSize("response data", data, 0)) return error;
		return in_place_valid;
	}
}


void encode(std::vector<std::uint8_t> & out, std::uint8_t id, ReadInt8Var      const & cmd) { return encodeReadVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, ReadInt16Var     const & cmd) { return encodeReadVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, ReadInt32Var     const & cmd) { return encodeReadVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, ReadFloat32Var   const & cmd) { return encodeReadVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, ReadPositionVar  const & cmd) { return encodeReadVar(out, id, cmd); }

void encode(std::vector<std::uint8_t> & out, std::uint8_t id, WriteInt8Var     const & cmd) { return encodeWriteVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, WriteInt16Var    const & cmd) { return encodeWriteVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, WriteInt32Var    const & cmd) { return encodeWriteVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, WriteFloat32Var  const & cmd) { return encodeWriteVar(out, id, cmd); }
void encode(std::vector<std::uint8_t> & out, std::uint8_t id, WritePositionVar const & cmd) { return encodeWriteVar(out, id, cmd); }

ErrorOr<std::vector<std::uint8_t>>  decode(ResponseHeader const &, string_view & data, ReadInt8Var      const & cmd) { return decodeReadVar(data, cmd); }
ErrorOr<std::vector<std::int16_t>>  decode(ResponseHeader const &, string_view & data, ReadInt16Var     const & cmd) { return decodeReadVar(data, cmd); }
ErrorOr<std::vector<std::int32_t>>  decode(ResponseHeader const &, string_view & data, ReadInt32Var     const & cmd) { return decodeReadVar(data, cmd); }
ErrorOr<std::vector<float>>         decode(ResponseHeader const &, string_view & data, ReadFloat32Var   const & cmd) { return decodeReadVar(data, cmd); }
ErrorOr<std::vector<Position>>      decode(ResponseHeader const &, string_view & data, ReadPositionVar  const & cmd) { return decodeReadVar(data, cmd); }

ErrorOr<void> decode(ResponseHeader const &, string_view & data, WriteInt8Var     const & cmd) { return decodeWriteVar(data, cmd); }
ErrorOr<void> decode(ResponseHeader const &, string_view & data, WriteInt16Var    const & cmd) { return decodeWriteVar(data, cmd); }
ErrorOr<void> decode(ResponseHeader const &, string_view & data, WriteInt32Var    const & cmd) { return decodeWriteVar(data, cmd); }
ErrorOr<void> decode(ResponseHeader const &, string_view & data, WriteFloat32Var  const & cmd) { return decodeWriteVar(data, cmd); }
ErrorOr<void> decode(ResponseHeader const &, string_view & data, WritePositionVar const & cmd) { return decodeWriteVar(data, cmd); }

/// Encode a ReadFileList command.
void encode(std::vector<std::uint8_t> & out, std::uint8_t request_id, ReadFileList const & command) {
	encode(out, makeFileRequestHeader(command.type.size(), commands::file::read_file_list, request_id));
	std::copy(command.type.begin(), command.type.end(), std::back_inserter(out));
}

/// Decode a ReadFileList response.
ErrorOr<std::vector<std::string>> decode(ResponseHeader const &, std::string && data, ReadFileList const &) {
	if (data.size() == 0) return std::vector<std::string>{};
	if (data.size() == 1) return malformedResponse("file list consist of exactly one byte");

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

/// Encode a ReadFile command.
void encode(std::vector<std::uint8_t> & out, std::uint8_t request_id, ReadFile const & command) {
	encode(out, makeFileRequestHeader(command.name.size(), commands::file::read_file, request_id));
	out.insert(out.end(), command.name.begin(), command.name.end());
}

/// Decode a ReadFile response.
ErrorOr<std::string> decode(ResponseHeader const &, std::string && data, ReadFile const &) {
	return std::move(data);
}

/// Encode a WriteFile command.
void encode(std::vector<std::uint8_t> & out, std::uint8_t request_id, WriteFile const & command) {
	encode(out, makeFileRequestHeader(command.name.size(), commands::file::write_file, request_id));
	out.insert(out.end(), command.name.begin(), command.name.end());
}

/// Decode a WriteFile response.
ErrorOr<void> decode(ResponseHeader const &, string_view & data, WriteFile const &) {
	if (auto error = checkSize("response data", data, 0)) return error;
	return in_place_valid;
}

/// Encode a DeleteFile command.
void encode(std::vector<std::uint8_t> & out, std::uint8_t request_id, DeleteFile const & command) {
	encode(out, makeFileRequestHeader(command.name.size(), commands::file::delete_file, request_id));
	out.insert(out.end(), command.name.begin(), command.name.end());
}

/// Decode a DeleteFile response.
ErrorOr<void> decode(ResponseHeader const &, string_view & data, DeleteFile const &) {
	if (auto error = checkSize("response data", data, 0)) return error;
	return in_place_valid;
}

}}}
