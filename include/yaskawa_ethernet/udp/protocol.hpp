#pragma once
#include "commands.hpp"
#include "message.hpp"
#include "../types.hpp"
#include "../error.hpp"
#include "../string_view.hpp"

#include <dr_error/error_or.hpp>

#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {

struct ReadCurrentRobotPosition {
	using type = PulsePosition;
	constexpr static std::size_t encoded_size = 11 * 4; // TODO: Documentation says 13*4 :/
	static ErrorOr<PulsePosition> decode(string_view & data);
};

struct ByteVariable {
	using type = std::uint8_t;
	constexpr static std::uint16_t command_single   = commands::robot::readwrite_int8_variable;
	constexpr static std::uint16_t command_multiple = commands::robot::readwrite_multiple_int8;
	constexpr static std::size_t encoded_size = 1;
	static void encode(std::vector<std::uint8_t> & output, std::uint8_t value);
	static ErrorOr<std::uint8_t> decode(string_view & data);
};

struct Int16Variable {
	using type = std::int16_t;
	constexpr static std::uint16_t command_single   = commands::robot::readwrite_int16_variable;
	constexpr static std::uint16_t command_multiple = commands::robot::readwrite_multiple_int16;
	constexpr static std::size_t encoded_size = 2;
	static void encode(std::vector<std::uint8_t> & output, type value);
	static ErrorOr<type> decode(string_view & data);
};

struct Int32Variable {
	using type = std::int32_t;
	constexpr static std::uint16_t command_single   = commands::robot::readwrite_int32_variable;
	constexpr static std::uint16_t command_multiple = commands::robot::readwrite_multiple_int32;
	constexpr static std::size_t encoded_size = 4;
	static void encode(std::vector<std::uint8_t> & output, std::int32_t value);
	static ErrorOr<std::int32_t> decode(string_view & data);
};

struct Float32Variable {
	using type = float;
	constexpr static std::uint16_t command_single   = commands::robot::readwrite_float_variable;
	constexpr static std::uint16_t command_multiple = commands::robot::readwrite_multiple_float;
	constexpr static std::size_t encoded_size = 4;
	static void encode(std::vector<std::uint8_t> & output, float value);
	static ErrorOr<float> decode(string_view & data);
};

struct PositionVariable {
	using type = Position;
	constexpr static std::uint16_t command_single   = commands::robot::readwrite_robot_position_variable;
	constexpr static std::uint16_t command_multiple = commands::robot::readwrite_multiple_robot_position;
	constexpr static std::size_t encoded_size = 13 * 4;
	static void encode(std::vector<std::uint8_t> & output, Position const & value);
	static ErrorOr<Position> decode(string_view & data);
};

struct ReadFileList {
	using type = std::vector<std::string>;
	static void encode(std::vector<std::uint8_t> & output, string_view type);
	static ErrorOr<std::vector<std::string>> decode(string_view & data);
};

struct ReadFile {
	using type = void;
	static void encode(std::vector<std::uint8_t> & output, string_view name);
	static ErrorOr<void> decode(string_view & data);
};

struct WriteFile {
	using type = void;
	static void encode(std::vector<std::uint8_t> & output, string_view name);
	static ErrorOr<void> decode(string_view & data);
};

struct DeleteFile {
	using type = void;
	static void encode(std::vector<std::uint8_t> & output, string_view name);
	static ErrorOr<void> decode(string_view & data);
};

struct FileData {
	using type = std::string;
	static void encode(std::vector<std::uint8_t> & output, string_view data);
	static ErrorOr<std::string> decode(string_view & data);
};


}}}
