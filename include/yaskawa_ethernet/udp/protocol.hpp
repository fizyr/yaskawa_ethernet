#pragma once
#include "commands.hpp"
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

DECLARE_COMMAND(ReadStatus);
DECLARE_COMMAND(ReadCurrentPosition);
DECLARE_COMMAND(MoveL);

DECLARE_COMMAND(ReadInt8Variable);
DECLARE_COMMAND(ReadInt16Variable);
DECLARE_COMMAND(ReadInt32Variable);
DECLARE_COMMAND(ReadFloat32Variable);
DECLARE_COMMAND(ReadPositionVariable);

DECLARE_COMMAND(WriteInt8Variable);
DECLARE_COMMAND(WriteInt16Variable);
DECLARE_COMMAND(WriteInt32Variable);
DECLARE_COMMAND(WriteFloat32Variable);
DECLARE_COMMAND(WritePositionVariable);

DECLARE_FILE_READ_COMMAND(ReadFileList);
DECLARE_FILE_READ_COMMAND(ReadFile);
DECLARE_COMMAND(WriteFile);
DECLARE_COMMAND(DeleteFile);

#undef DECLARE_COMMAND

}}}
