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

struct ReadInt8Var {
	using Response = std::vector<std::uint8_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadInt16Var {
	using Response = std::vector<std::int16_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadInt32Var {
	using Response = std::vector<std::int32_t>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadFloat32Var {
	using Response = std::vector<float>;
	std::uint8_t index;
	std::uint8_t count;
};

struct ReadPositionVar {
	using Response = std::vector<Position>;
	std::uint8_t index;
	std::uint8_t count;
};

struct WriteInt8Var {
	using Response = void;
	std::uint8_t index;
	std::vector<std::uint8_t> values;
};

struct WriteInt16Var {
	using Response = void;
	std::uint8_t index;
	std::vector<std::int16_t> values;
};

struct WriteInt32Var {
	using Response = void;
	std::uint8_t index;
	std::vector<std::int32_t> values;
};

struct WriteFloat32Var {
	using Response = void;
	std::uint8_t index;
	std::vector<float> values;
};

struct WritePositionVar {
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
