#include "./commands.hpp"
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
template<> struct encoded_size<ReadInt8Variable>      : size_constant<1>{};
template<> struct encoded_size<WriteInt8Variable>     : size_constant<1>{};
template<> struct encoded_size<ReadInt16Variable>     : size_constant<2>{};
template<> struct encoded_size<WriteInt16Variable>    : size_constant<2>{};
template<> struct encoded_size<ReadInt32Variable>     : size_constant<4>{};
template<> struct encoded_size<WriteInt32Variable>    : size_constant<4>{};
template<> struct encoded_size<ReadFloat32Variable>   : size_constant<4>{};
template<> struct encoded_size<WriteFloat32Variable>  : size_constant<4>{};
template<> struct encoded_size<ReadPositionVariable>  : size_constant<13 * 4>{};
template<> struct encoded_size<WritePositionVariable> : size_constant<13 * 4>{};

/// The UDP command for reading/writing a single value.
template<typename Command> struct single_command;
template<> struct single_command<ReadInt8Variable>      : command_constant<commands::robot::readwrite_int8_variable>{};
template<> struct single_command<WriteInt8Variable>     : command_constant<commands::robot::readwrite_int8_variable>{};
template<> struct single_command<ReadInt16Variable>     : command_constant<commands::robot::readwrite_int16_variable>{};
template<> struct single_command<WriteInt16Variable>    : command_constant<commands::robot::readwrite_int16_variable>{};
template<> struct single_command<ReadInt32Variable>     : command_constant<commands::robot::readwrite_int32_variable>{};
template<> struct single_command<WriteInt32Variable>    : command_constant<commands::robot::readwrite_int32_variable>{};
template<> struct single_command<ReadFloat32Variable>   : command_constant<commands::robot::readwrite_float_variable>{};
template<> struct single_command<WriteFloat32Variable>  : command_constant<commands::robot::readwrite_float_variable>{};
template<> struct single_command<ReadPositionVariable>  : command_constant<commands::robot::readwrite_robot_position_variable>{};
template<> struct single_command<WritePositionVariable> : command_constant<commands::robot::readwrite_robot_position_variable>{};

/// The UDP command for reading/writing a multiple values.
template<typename Command> struct multi_command;
template<> struct multi_command<ReadInt8Variable>      : command_constant<commands::robot::readwrite_multiple_int8>{};
template<> struct multi_command<WriteInt8Variable>     : command_constant<commands::robot::readwrite_multiple_int8>{};
template<> struct multi_command<ReadInt16Variable>     : command_constant<commands::robot::readwrite_multiple_int16>{};
template<> struct multi_command<WriteInt16Variable>    : command_constant<commands::robot::readwrite_multiple_int16>{};
template<> struct multi_command<ReadInt32Variable>     : command_constant<commands::robot::readwrite_multiple_int32>{};
template<> struct multi_command<WriteInt32Variable>    : command_constant<commands::robot::readwrite_multiple_int32>{};
template<> struct multi_command<ReadFloat32Variable>   : command_constant<commands::robot::readwrite_multiple_float>{};
template<> struct multi_command<WriteFloat32Variable>  : command_constant<commands::robot::readwrite_multiple_float>{};
template<> struct multi_command<ReadPositionVariable>  : command_constant<commands::robot::readwrite_multiple_robot_position>{};
template<> struct multi_command<WritePositionVariable> : command_constant<commands::robot::readwrite_multiple_robot_position>{};

/// If true, Command is a read command taking and index and a count, and returning a vector of elements.
template<typename Command> struct is_read_command : std::false_type{};
template<> struct is_read_command<ReadInt8Variable>      : std::true_type{};
template<> struct is_read_command<ReadInt16Variable>     : std::true_type{};
template<> struct is_read_command<ReadInt32Variable>     : std::true_type{};
template<> struct is_read_command<ReadFloat32Variable>   : std::true_type{};
template<> struct is_read_command<ReadPositionVariable>  : std::true_type{};

/// If true, Command is a write command taking an index and a vector of elements, and returning void.
template<typename Command> struct is_write_command : std::false_type{};
template<> struct is_write_command<WriteInt8Variable>      : std::true_type{};
template<> struct is_write_command<WriteInt16Variable>     : std::true_type{};
template<> struct is_write_command<WriteInt32Variable>     : std::true_type{};
template<> struct is_write_command<WriteFloat32Variable>   : std::true_type{};
template<> struct is_write_command<WritePositionVariable>  : std::true_type{};

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
