#pragma once
#include <cstdint>
namespace dr {
namespace yaskawa {
namespace udp {

enum class Division {
	robot = 1,
	file  = 2,
};

namespace service {
	constexpr std::uint8_t get_single     = 0x0e;
	constexpr std::uint8_t set_single     = 0x10;
	constexpr std::uint8_t get_all        = 0x01;
	constexpr std::uint8_t set_all        = 0x02;
	constexpr std::uint8_t read_multiple  = 0x33;
	constexpr std::uint8_t write_multiple = 0x34;
}

struct Header {
	std::uint16_t payload_size;
	Division division;
	bool ack;
	std::uint8_t request_id;
	std::uint32_t block_number;
};

constexpr std::size_t header_size      = 0x20;
constexpr std::size_t max_payload_size = 0x479;

struct RequestHeader : Header {
	std::uint16_t command;
	std::uint16_t instance;
	std::uint8_t  attribute;
	std::uint8_t  service;
};

struct ResponseHeader : Header {
	std::uint8_t service;
	std::uint8_t status;
	std::uint16_t extra_status;
};

}}}
