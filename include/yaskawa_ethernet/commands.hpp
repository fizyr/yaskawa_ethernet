/* Copyright 2016-2019 Fizyr B.V. - https://fizyr.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
