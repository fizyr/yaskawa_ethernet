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

#include "./message.hpp"
#include "../commands.hpp"

#include <estd/result.hpp>

#include <type_traits>

namespace dr {
namespace yaskawa {
namespace udp {

template<std::uint16_t value> using command_constant = std::integral_constant<std::uint16_t, value>;
template<std::size_t   value> using    size_constant = std::integral_constant<std::size_t, value>;
template<bool          value> using    bool_constant = std::integral_constant<bool, value>;

/// The encoded size of a variable for the UDP protocol.
template<typename T>       struct encoded_size;

/// The command number for the UDP protocol.
template<typename Command> struct udp_command;

#define VAR_TRAITS(TYPE, SIZE, SINGLE, MULTI) \
template<> struct encoded_size<TYPE> : size_constant<SIZE> {}; \
template<> struct udp_command<ReadVar<TYPE>>   : command_constant<SINGLE>{}; \
template<> struct udp_command<WriteVar<TYPE>>  : command_constant<SINGLE>{}; \
template<> struct udp_command<ReadVars<TYPE>>   : command_constant<MULTI>{}; \
template<> struct udp_command<WriteVars<TYPE>>  : command_constant<MULTI>{}

VAR_TRAITS(std::uint8_t,     1, commands::robot::readwrite_int8_variable,           commands::robot::readwrite_multiple_int8);
VAR_TRAITS(std::int16_t,     2, commands::robot::readwrite_int16_variable,          commands::robot::readwrite_multiple_int16);
VAR_TRAITS(std::int32_t,     4, commands::robot::readwrite_int32_variable,          commands::robot::readwrite_multiple_int32);
VAR_TRAITS(float,            4, commands::robot::readwrite_float_variable,          commands::robot::readwrite_multiple_float);
VAR_TRAITS(Position,     13* 4, commands::robot::readwrite_robot_position_variable, commands::robot::readwrite_multiple_robot_position);

/// If true, Command is a multi-part download command.
template<typename Command> struct is_file_read_command : std::false_type{};
template<> struct is_file_read_command<ReadFileList>  : std::true_type{};
template<> struct is_file_read_command<ReadFile>      : std::true_type{};

/// If true, Command is a multi-part upload command.
template<typename Command> struct is_file_write_command : std::false_type{};
template<> struct is_file_read_command<WriteFile> : std::true_type{};

/// If true, Command is a multi-part upload or download command.
template<typename Command> struct is_file_command : bool_constant<false
	|| is_file_read_command<Command>::value
	|| is_file_write_command<Command>::value
> {};

}}}
