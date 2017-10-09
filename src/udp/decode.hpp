#pragma once
#include "error.hpp"
#include "string_view.hpp"
#include "types.hpp"
#include "udp/message.hpp"

#include <dr_error/error_or.hpp>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace dr {
namespace yaskawa {
namespace udp {

/// Read little-endian integral data from a byte buffer.
template<typename T>
T readLittleEndian(std::uint8_t const * data) {
	static_assert(std::is_integral<T>::value, "T must be an integral type.");
	T result = 0;
	for (unsigned int i = 0; i < sizeof(T); ++i) {
		result |= data[i] << i * 8;
	}
	return result;
}

/// Read and remove little-endian integral data from the front of a string view.
template<typename T>
T readLittleEndian(string_view & data) {
	data.remove_prefix(sizeof(T));
	return readLittleEndian<T>(reinterpret_cast<std::uint8_t const *>(data.data() - sizeof(T)));
}

/// Decode a response header.
ErrorOr<ResponseHeader> decodeResponseHeader(string_view & data);

/// Generic decode function for raw types.
template<typename T>
ErrorOr<T> decode(string_view & data);

template<> ErrorOr<std::uint8_t> decode<std::uint8_t>(string_view & data);
template<> ErrorOr<std::int16_t> decode<std::int16_t>(string_view & data);
template<> ErrorOr<std::int32_t> decode<std::int32_t>(string_view & data);
template<> ErrorOr<float> decode<float>(string_view & data);
template<> ErrorOr<Position> decode<Position>(string_view & data);
template<> ErrorOr<PulsePosition> decode<PulsePosition>(string_view & data);
template<> ErrorOr<CartesianPosition> decode<CartesianPosition>(string_view & data);

}}}
