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

/// The encoded size of a single element for the UDP protocol.
template<typename Command> struct encoded_size;
template<> struct encoded_size<ReadInt8Var>      : size_constant<1>{};
template<> struct encoded_size<WriteInt8Var>     : size_constant<1>{};
template<> struct encoded_size<ReadInt16Var>     : size_constant<2>{};
template<> struct encoded_size<WriteInt16Var>    : size_constant<2>{};
template<> struct encoded_size<ReadInt32Var>     : size_constant<4>{};
template<> struct encoded_size<WriteInt32Var>    : size_constant<4>{};
template<> struct encoded_size<ReadFloat32Var>   : size_constant<4>{};
template<> struct encoded_size<WriteFloat32Var>  : size_constant<4>{};
template<> struct encoded_size<ReadPositionVar>  : size_constant<13 * 4>{};
template<> struct encoded_size<WritePositionVar> : size_constant<13 * 4>{};

/// The UDP command for reading/writing a single value.
template<typename Command> struct single_command;
template<> struct single_command<ReadInt8Var>      : command_constant<commands::robot::readwrite_int8_variable>{};
template<> struct single_command<WriteInt8Var>     : command_constant<commands::robot::readwrite_int8_variable>{};
template<> struct single_command<ReadInt16Var>     : command_constant<commands::robot::readwrite_int16_variable>{};
template<> struct single_command<WriteInt16Var>    : command_constant<commands::robot::readwrite_int16_variable>{};
template<> struct single_command<ReadInt32Var>     : command_constant<commands::robot::readwrite_int32_variable>{};
template<> struct single_command<WriteInt32Var>    : command_constant<commands::robot::readwrite_int32_variable>{};
template<> struct single_command<ReadFloat32Var>   : command_constant<commands::robot::readwrite_float_variable>{};
template<> struct single_command<WriteFloat32Var>  : command_constant<commands::robot::readwrite_float_variable>{};
template<> struct single_command<ReadPositionVar>  : command_constant<commands::robot::readwrite_robot_position_variable>{};
template<> struct single_command<WritePositionVar> : command_constant<commands::robot::readwrite_robot_position_variable>{};

/// The UDP command for reading/writing a multiple values.
template<typename Command> struct multi_command;
template<> struct multi_command<ReadInt8Var>      : command_constant<commands::robot::readwrite_multiple_int8>{};
template<> struct multi_command<WriteInt8Var>     : command_constant<commands::robot::readwrite_multiple_int8>{};
template<> struct multi_command<ReadInt16Var>     : command_constant<commands::robot::readwrite_multiple_int16>{};
template<> struct multi_command<WriteInt16Var>    : command_constant<commands::robot::readwrite_multiple_int16>{};
template<> struct multi_command<ReadInt32Var>     : command_constant<commands::robot::readwrite_multiple_int32>{};
template<> struct multi_command<WriteInt32Var>    : command_constant<commands::robot::readwrite_multiple_int32>{};
template<> struct multi_command<ReadFloat32Var>   : command_constant<commands::robot::readwrite_multiple_float>{};
template<> struct multi_command<WriteFloat32Var>  : command_constant<commands::robot::readwrite_multiple_float>{};
template<> struct multi_command<ReadPositionVar>  : command_constant<commands::robot::readwrite_multiple_robot_position>{};
template<> struct multi_command<WritePositionVar> : command_constant<commands::robot::readwrite_multiple_robot_position>{};

/// If true, Command is a read command taking and index and a count, and returning a vector of elements.
template<typename Command> struct is_read_command : std::false_type{};
template<> struct is_read_command<ReadInt8Var>      : std::true_type{};
template<> struct is_read_command<ReadInt16Var>     : std::true_type{};
template<> struct is_read_command<ReadInt32Var>     : std::true_type{};
template<> struct is_read_command<ReadFloat32Var>   : std::true_type{};
template<> struct is_read_command<ReadPositionVar>  : std::true_type{};

/// If true, Command is a write command taking an index and a vector of elements, and returning void.
template<typename Command> struct is_write_command : std::false_type{};
template<> struct is_write_command<WriteInt8Var>      : std::true_type{};
template<> struct is_write_command<WriteInt16Var>     : std::true_type{};
template<> struct is_write_command<WriteInt32Var>     : std::true_type{};
template<> struct is_write_command<WriteFloat32Var>   : std::true_type{};
template<> struct is_write_command<WritePositionVar>  : std::true_type{};

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
