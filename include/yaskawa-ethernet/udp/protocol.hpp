#pragma once
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"

#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace udp {

template<typename T> std::vector<std::uint8_t> encode(T const & message, std::uint8_t request_id = 0);
template<typename T> ErrorOr<T> decode(string_view message);

template<> std::vector<std::uint8_t> encode<ReadInt8Variable::Request>(ReadInt8Variable::Request const &, std::uint8_t request_id);
template<> ErrorOr<ReadInt8Variable::Response> decode<ReadInt8Variable::Response>(string_view message);

template<> std::vector<std::uint8_t> encode<WriteInt8Variable::Request>(WriteInt8Variable::Request const &, std::uint8_t request_id);
template<> ErrorOr<WriteInt8Variable::Response> decode<WriteInt8Variable::Response>(string_view message);

}}}
