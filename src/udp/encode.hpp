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
#include "udp/message.hpp"

#include <cstdint>
#include <type_traits>
#include <vector>

namespace dr {
namespace yaskawa {
namespace udp {

RequestHeader makeRobotRequestHeader(
	std::uint16_t payload_size,
	std::uint16_t command,
	std::uint16_t instance,
	std::uint8_t attribute,
	std::uint8_t service,
	std::uint8_t request_id
);

RequestHeader makeFileRequestHeader(
	std::uint16_t payload_size,
	std::uint8_t service,
	std::uint8_t request_id,
	std::uint32_t block_number = 0,
	bool ack = false
);

template<typename T>
void writeLittleEndian(std::vector<std::uint8_t> & out, T value) {
	static_assert(std::is_integral<T>::value, "T must be an integral type.");
	for (unsigned int i = 0; i < sizeof(T); ++i) {
		out.push_back(value >> i * 8 & 0xff);
	}
}

void encode(std::vector<std::uint8_t> & out, RequestHeader const & header);
void encode(std::vector<std::uint8_t> & out, std::uint8_t value);
void encode(std::vector<std::uint8_t> & out, std::int16_t value);
void encode(std::vector<std::uint8_t> & out, std::int32_t value);
void encode(std::vector<std::uint8_t> & out, float value);
void encode(std::vector<std::uint8_t> & out, PulsePosition const & position);
void encode(std::vector<std::uint8_t> & out, CartesianPosition const & position);
void encode(std::vector<std::uint8_t> & out, Position const & position);

}}}
