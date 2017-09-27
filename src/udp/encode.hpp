#pragma once
#include "types.hpp"
#include "udp/commands.hpp"
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
