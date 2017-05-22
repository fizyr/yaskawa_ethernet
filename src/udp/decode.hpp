#include "error.hpp"
#include "string_view.hpp"
#include "types.hpp"
#include "udp/commands.hpp"
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

DetailedError malformedResponse(std::string message);

DetailedError maximumExceeded(std::string field, int value, int max);

DetailedError unexpectedValue(std::string field, int value, int expected);

DetailedError commandFailed(std::uint16_t status, std::uint16_t extra_status);

/// Decode a response header.
ErrorOr<ResponseHeader> decodeResponseHeader(string_view & data);

/// Decode a cartesian frame from a position type and a user frame.
ErrorOr<CoordinateSystem> decodeCartesianFrame(int type, int user_frame);

/// Decode an integral value.
template<typename T>
ErrorOr<typename T::type> decodeIntegral(string_view message, std::string name) {
	if (message.size() != T::encoded_size) return unexpectedValue(std::move(name) + " size", message.size(), T::encoded_size);
	return readLittleEndian<typename T::type>(message);
}

/// Decode a read response.
template<typename T>
ErrorOr<typename T::type> decodeReadResponse(string_view response) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(response);
	if (!header) return header.error();
	return T::decode(response);
}

/// Decode a read response.
template<typename T>
ErrorOr<std::vector<typename T::type>> decodeReadMultipleResponse(string_view response) {
	using type = typename T::type;

	ErrorOr<ResponseHeader> header = decodeResponseHeader(response);
	if (!header) return header.error();
	if (response.size() < 4) return malformedResponse("payload size too small, got " + std::to_string(response.size()) + ", need atleast 4.");
	std::uint32_t count = readLittleEndian<std::uint32_t>(response);
	if (response.size() != count * T::encoded_size) return unexpectedValue("data size", response.size(), count * T::encoded_size);

	std::vector<type> result;
	result.reserve(count);
	for (std::size_t i = 0; i < count; ++i) {
		ErrorOr<type> element = T::decode(response);
		if (!element) return element.error();
		result.push_back(std::move(*element));
	}
	return result;
}

/// Decode a write response.
template<typename T>
ErrorOr<void> decodeWriteResponse(string_view response) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(response);
	if (!header) return header.error();
	return in_place_valid;
}

}}}
