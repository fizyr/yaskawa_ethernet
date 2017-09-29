#include "./message.hpp"
#include "../commands.hpp"
#include "../string_view.hpp"

#include <dr_error/error_or.hpp>

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

VAR_TRAITS(std::uint8_t, 1, commands::robot::readwrite_int8_variable,           commands::robot::readwrite_multiple_int8);
VAR_TRAITS(std::int16_t, 2, commands::robot::readwrite_int16_variable,          commands::robot::readwrite_multiple_int16);
VAR_TRAITS(std::int32_t, 4, commands::robot::readwrite_int32_variable,          commands::robot::readwrite_multiple_int32);
VAR_TRAITS(float,        4, commands::robot::readwrite_float_variable,          commands::robot::readwrite_multiple_float);
VAR_TRAITS(Position, 13* 4, commands::robot::readwrite_robot_position_variable, commands::robot::readwrite_multiple_robot_position);

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
