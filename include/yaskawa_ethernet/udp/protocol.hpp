#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"

#include <dr_error/error_or.hpp>

#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {

std::vector<std::uint8_t> encodeReadVariable(std::uint8_t request_id, int index, std::uint16_t command);
ErrorOr<void> decodeWriteVariable(string_view message);

ErrorOr<std::uint8_t> decodeReadInt8Variable(string_view message);
std::vector<std::uint8_t> encodeWriteInt8Variable(std::uint8_t request_id, int index, std::uint8_t value);

ErrorOr<std::int16_t> decodeReadInt16Variable(string_view message);
std::vector<std::uint8_t> encodeWriteInt16Variable(std::uint8_t request_id, int index, std::int16_t value);

ErrorOr<std::int32_t> decodeReadInt32Variable(string_view message);
std::vector<std::uint8_t> encodeWriteInt32Variable(std::uint8_t request_id, int index, std::int32_t value);

ErrorOr<float> decodeReadFloat32Variable(string_view message);
std::vector<std::uint8_t> encodeWriteFloat32Variable(std::uint8_t request_id, int index, float value);

ErrorOr<Position> decodeReadPositionVariable(string_view message);
std::vector<std::uint8_t> encodeWritePositionVariable(std::uint8_t request_id, int index, Position const & value, std::uint16_t command);

}}}
