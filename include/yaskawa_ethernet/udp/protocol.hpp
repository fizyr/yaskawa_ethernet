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
#include "message.hpp"
#include "../commands.hpp"
#include "../error.hpp"
#include "../types.hpp"

#include <estd/result.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

namespace dr {
namespace yaskawa {
namespace udp {

// Macro to declare encode/decode functions in the dr::yaskawa::udp namespace.
#define DECLARE_COMMAND(TYPE) \
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, TYPE const & command); \
Result<TYPE::Response> decode(ResponseHeader const & header, std::string_view & data, TYPE const & command)

// File read functions get a string && owning the data to prevent needless copying.
#define DECLARE_FILE_READ_COMMAND(TYPE) \
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, TYPE const & command); \
Result<TYPE::Response> decode(ResponseHeader const & header, std::string && data, TYPE const & command)

// Declare ReadVar<TYPE>, ReadVars<TYPE>, WriteVar<TYPE> and WriteVars<TYPE> commands.
#define DECLARE_VAR(TYPE) \
DECLARE_COMMAND(ReadVar<TYPE>); \
DECLARE_COMMAND(ReadVars<TYPE>); \
DECLARE_COMMAND(WriteVar<TYPE>); \
DECLARE_COMMAND(WriteVars<TYPE>)

DECLARE_COMMAND(ReadStatus);
DECLARE_COMMAND(ReadCurrentPosition);
DECLARE_COMMAND(MoveL);

DECLARE_VAR(std::uint8_t);
DECLARE_VAR(std::int16_t);
DECLARE_VAR(std::int32_t);
DECLARE_VAR(float);
DECLARE_VAR(Position);

DECLARE_FILE_READ_COMMAND(ReadFileList);
DECLARE_FILE_READ_COMMAND(ReadFile);
DECLARE_COMMAND(WriteFile);
DECLARE_COMMAND(DeleteFile);

#undef DECLARE_COMMAND
#undef DECLARE_FILE_READ_COMMAND
#undef DECLARE_VAR

}}}
