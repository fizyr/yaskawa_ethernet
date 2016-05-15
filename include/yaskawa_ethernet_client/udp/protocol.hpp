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

template<> std::vector<std::uint8_t> encode<ReadByteVariable::Request>(ReadByteVariable::Request const &, std::uint8_t request_id);
template<> ErrorOr<ReadByteVariable::Response> decode<ReadByteVariable::Response>(string_view message);

template<> std::vector<std::uint8_t> encode<WriteByteVariable::Request>(WriteByteVariable::Request const &, std::uint8_t request_id);
template<> ErrorOr<WriteByteVariable::Response> decode<WriteByteVariable::Response>(string_view message);

}}}
