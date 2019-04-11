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
#include <estd/result.hpp>

#include <string>
#include <system_error>
#include <type_traits>

namespace dr {
namespace yaskawa {

/// Get a reference to the Yaskawa error category.
std::error_category const & yaskawa_category();

/// Protocol error code constants.
namespace errc {
	enum errc_t {
		malformed_response  = 0x01,
		command_failed      = 0x02,
		unknown_request     = 0x03,
	};

	inline std::error_code      make_error_code(errc_t code)      { return {code, yaskawa_category()}; }
	inline std::error_condition make_error_condition(errc_t code) { return {code, yaskawa_category()}; }
}

/// Enum type for protocol error codes.
using errc_t = errc::errc_t;

estd::error malformedResponse(std::string message);
estd::error commandFailed(std::uint16_t status, std::uint16_t extra_status);

estd::error expectValue(std::string name, int value, int expected);
estd::error expectValueMin(std::string name, int value, int min);
estd::error expectValueMax(std::string name, int value, int max);
estd::error expectValueMinMax(std::string name, int value, int min, int max);
estd::error expectSize(std::string description, std::size_t actual_size, std::size_t expected_size);
estd::error expectSizeMin(std::string description, std::size_t actual_size, std::size_t minimum_size);
estd::error expectSizeMax(std::string description, std::size_t actual_size, std::size_t maximum_size);
estd::error expectSizeMinMax(std::string description, std::size_t actual_size, std::size_t maximum_size);

}}

namespace std {
	template<> class is_error_code_enum     <dr::yaskawa::errc_t> : std::true_type {};
	template<> class is_error_condition_enum<dr::yaskawa::errc_t> : std::true_type {};
}
