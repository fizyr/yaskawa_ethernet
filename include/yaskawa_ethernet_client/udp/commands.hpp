#pragma once
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {
namespace commands {

namespace robot {
	std::uint16_t read_alarm                        = 0x70;
	std::uint16_t read_alarm_history                = 0x71;
	std::uint16_t read_status_information           = 0x72;
	std::uint16_t execute_job_information           = 0x73;
	std::uint16_t read_axis_configuration           = 0x74;
	std::uint16_t read_robot_position               = 0x75;
	std::uint16_t read_position_error               = 0x76;
	std::uint16_t read_torque                       = 0x77;
	std::uint16_t read_io                           = 0x78;
	std::uint16_t read_register                     = 0x79;
	std::uint16_t read_int8_variable                = 0x7a;
	std::uint16_t read_int16_variable               = 0x7b;
	std::uint16_t read_int32_variable               = 0x7c;
	std::uint16_t read_float_variable               = 0x7d;
	std::uint16_t read_string_variable              = 0x7e;
	std::uint16_t read_robot_position_variable      = 0x7f;
	std::uint16_t read_base_position_variable       = 0x80;
	std::uint16_t read_external_axis_variable       = 0x81;
	std::uint16_t reset_alarm                       = 0x82;
	std::uint16_t set_servo_enabled                 = 0x83;
	std::uint16_t set_execution_mode                = 0x84;
	std::uint16_t show_message                      = 0x85;
	std::uint16_t start_job                         = 0x86;
	std::uint16_t select_job                        = 0x87;
	std::uint16_t read_management_time              = 0x88;
	std::uint16_t read_system_information           = 0x89;
	std::uint16_t move_cartesian                    = 0x8a;
	std::uint16_t move_pulse                        = 0x8b;
	std::uint16_t readwrite_multiple_io             = 0x300;
	std::uint16_t readwrite_multiple_register       = 0x301;
	std::uint16_t readwrite_multiple_int8           = 0x302;
	std::uint16_t readwrite_multiple_int16          = 0x303;
	std::uint16_t readwrite_multiple_int32          = 0x304;
	std::uint16_t readwrite_multiple_float          = 0x305;
	std::uint16_t readwrite_multiple_string         = 0x306;
	std::uint16_t readwrite_multiple_robot_position = 0x307;
	std::uint16_t readwrite_multiple_base_position  = 0x308;
	std::uint16_t readwrite_multiple_external_axis  = 0x309;
	std::uint16_t read_alarm_data                   = 0x30a;
}

namespace file {
	std::uint8_t delete_file    = 0x09;
	std::uint8_t write_file     = 0x15;
	std::uint8_t read_file      = 0x16;
	std::uint8_t read_file_list = 0x32;
}

}}}}
