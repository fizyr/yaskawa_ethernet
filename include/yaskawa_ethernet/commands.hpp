#pragma once
#include "types.hpp"

#include <cstdint>
#include <vector>

namespace dr {
namespace yaskawa {

struct ReadStatus {
	using Response = Status;
};

struct ReadCurrentPosition {
	using Response = Position;
	int control_group;
	CoordinateSystemType coordinate_system;
};

struct MoveL {
	using Response = void;
	int control_group;
	CartesianPosition const & target;
	Speed speed;
};

struct ReadInt8Variable {
	using Response = std::vector<std::uint8_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadInt16Variable {
	using Response = std::vector<std::int16_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadInt32Variable {
	using Response = std::vector<std::int32_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadFloat32Variable {
	using Response = std::vector<float>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadPositionVariable {
	using Response = std::vector<Position>;
	std::uint8_t index;
	std::uint8_t count;
};

struct WriteInt8Variable {
	using Response = void;
	std::uint8_t index;
	std::vector<std::uint8_t> values;
};

struct WriteInt16Variable {
	using Response = void;
	std::uint8_t index;
	std::vector<std::int16_t> values;
};

struct WriteInt32Variable {
	using Response = void;
	std::uint8_t index;
	std::vector<std::int32_t> values;
};

struct WriteFloat32Variable {
	using Response = void;
	std::uint8_t index;
	std::vector<float> values;
};

struct WritePositionVariable {
	using Response = void;
	std::uint8_t index;
	std::vector<Position> values;
};

struct ReadFileList {
	using Response = std::vector<std::string>;
	std::string type;
};

struct ReadFile {
	using Response = std::string;
	std::string name;
};

struct WriteFile {
	using Response = void;
	std::string name;
	std::string data;
};

struct DeleteFile {
	using Response = void;
	std::string name;
};

}}
