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

/// Read a variable from the robot.
/**
 * Currently supported types:
 * std::uint8_t (B variable)
 * std::int16_t (I variable)
 * std::int32_t (D variable)
 * float        (R variable)
 * Position     (P variable)
 */
template<typename T>
struct ReadVar {
	using Response = T;
	std::uint8_t index;
};

/// Read a sequence of variables from the robot.
/**
 * See ReadVar<T> for a list of supported types.
 */
template<typename T>
struct ReadVars {
	using Response = std::vector<T>;
	std::uint8_t index;
	std::uint8_t count;
};

/// Write a variable from the robot.
/**
 * See ReadVar<T> for a list of supported types.
 */
template<typename T>
struct WriteVar {
	using Response = void;
	std::uint8_t index;
	T value;
};

/// Read a variable from the robot.
/**
 * See ReadVar<T> for a list of supported types.
 */
template<typename T>
struct WriteVars {
	using Response = void;
	std::uint8_t index;
	std::vector<T> values;
};

using ReadUint8Var   = ReadVar   <std::uint8_t>;
using ReadUint8Vars  = ReadVars  <std::uint8_t>;
using WriteUint8Var  = WriteVar  <std::uint8_t>;
using WriteUint8Vars = WriteVars <std::uint8_t>;

using ReadInt16Var   = ReadVar   <std::int16_t>;
using ReadInt16Vars  = ReadVars  <std::int16_t>;
using WriteInt16Var  = WriteVar  <std::int16_t>;
using WriteInt16Vars = WriteVars <std::int16_t>;

using ReadInt32Var   = ReadVar   <std::int32_t>;
using ReadInt32Vars  = ReadVars  <std::int32_t>;
using WriteInt32Var  = WriteVar  <std::int32_t>;
using WriteInt32Vars = WriteVars <std::int32_t>;

using ReadFloat32Var   = ReadVar   <float>;
using ReadFloat32Vars  = ReadVars  <float>;
using WriteFloat32Var  = WriteVar  <float>;
using WriteFloat32Vars = WriteVars <float>;

using ReadPositionVar   = ReadVar   <Position>;
using ReadPositionVars  = ReadVars  <Position>;
using WritePositionVar  = WriteVar  <Position>;
using WritePositionVars = WriteVars <Position>;

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
