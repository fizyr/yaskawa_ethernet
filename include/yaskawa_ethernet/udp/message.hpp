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
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {

namespace commands {
	namespace robot {
		constexpr std::uint16_t read_alarm                        = 0x70;
		constexpr std::uint16_t read_alarm_history                = 0x71;
		constexpr std::uint16_t read_status_information           = 0x72;
		constexpr std::uint16_t execute_job_information           = 0x73;
		constexpr std::uint16_t read_axis_configuration           = 0x74;
		constexpr std::uint16_t read_robot_position               = 0x75;
		constexpr std::uint16_t read_position_error               = 0x76;
		constexpr std::uint16_t read_torque                       = 0x77;
		constexpr std::uint16_t readwrite_io                      = 0x78;
		constexpr std::uint16_t readwrite_register                = 0x79;
		constexpr std::uint16_t readwrite_int8_variable           = 0x7a;
		constexpr std::uint16_t readwrite_int16_variable          = 0x7b;
		constexpr std::uint16_t readwrite_int32_variable          = 0x7c;
		constexpr std::uint16_t readwrite_float_variable          = 0x7d;
		constexpr std::uint16_t readwrite_string_variable         = 0x7e;
		constexpr std::uint16_t readwrite_robot_position_variable = 0x7f;
		constexpr std::uint16_t readwrite_base_position_variable  = 0x80;
		constexpr std::uint16_t readwrite_external_axis_variable  = 0x81;
		constexpr std::uint16_t reset_alarm                       = 0x82;
		constexpr std::uint16_t set_servo_enabled                 = 0x83;
		constexpr std::uint16_t set_execution_mode                = 0x84;
		constexpr std::uint16_t show_message                      = 0x85;
		constexpr std::uint16_t start_job                         = 0x86;
		constexpr std::uint16_t select_job                        = 0x87;
		constexpr std::uint16_t read_management_time              = 0x88;
		constexpr std::uint16_t read_system_information           = 0x89;
		constexpr std::uint16_t move_cartesian                    = 0x8a;
		constexpr std::uint16_t move_pulse                        = 0x8b;
		constexpr std::uint16_t readwrite_multiple_io             = 0x300;
		constexpr std::uint16_t readwrite_multiple_register       = 0x301;
		constexpr std::uint16_t readwrite_multiple_int8           = 0x302;
		constexpr std::uint16_t readwrite_multiple_int16          = 0x303;
		constexpr std::uint16_t readwrite_multiple_int32          = 0x304;
		constexpr std::uint16_t readwrite_multiple_float          = 0x305;
		constexpr std::uint16_t readwrite_multiple_string         = 0x306;
		constexpr std::uint16_t readwrite_multiple_robot_position = 0x307;
		constexpr std::uint16_t readwrite_multiple_base_position  = 0x308;
		constexpr std::uint16_t readwrite_multiple_external_axis  = 0x309;
		constexpr std::uint16_t read_alarm_data                   = 0x30a;
	}

	namespace file {
		constexpr std::uint8_t delete_file    = 0x09;
		constexpr std::uint8_t write_file     = 0x15;
		constexpr std::uint8_t read_file      = 0x16;
		constexpr std::uint8_t read_file_list = 0x32;
	}
}

enum class Division {
	robot = 1,
	file  = 2,
};

namespace service {
	constexpr std::uint8_t get_single     = 0x0e;
	constexpr std::uint8_t set_single     = 0x10;
	constexpr std::uint8_t get_all        = 0x01;
	constexpr std::uint8_t set_all        = 0x02;
	constexpr std::uint8_t read_multiple  = 0x33;
	constexpr std::uint8_t write_multiple = 0x34;
}

struct Header {
	std::uint16_t payload_size;
	Division division;
	bool ack;
	std::uint8_t request_id;
	std::uint32_t block_number;
};

constexpr std::size_t header_size      = 0x20;
constexpr std::size_t max_payload_size = 0x479;

struct RequestHeader : Header {
	std::uint16_t command;
	std::uint16_t instance;
	std::uint8_t  attribute;
	std::uint8_t  service;
};

struct ResponseHeader : Header {
	std::uint8_t service;
	std::uint8_t status;
	std::uint16_t extra_status;
};

}}}
