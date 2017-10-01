#pragma once
#include "message.hpp"
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"
#include "../types.hpp"

#include <dr_error/error_or.hpp>

#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {

// Macro to declare encode/decode functions in the dr::yaskawa::udp namespace.
#define DECLARE_COMMAND(TYPE) \
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, TYPE const & command); \
ErrorOr<TYPE::Response> decode(ResponseHeader const & header, string_view & data, TYPE const & command)

// File read functions get a string && owning the data to prevent needless copying.
#define DECLARE_FILE_READ_COMMAND(TYPE) \
void encode(std::vector<std::uint8_t> & output, std::uint8_t request_id, TYPE const & command); \
ErrorOr<TYPE::Response> decode(ResponseHeader const & header, std::string && data, TYPE const & command)

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
