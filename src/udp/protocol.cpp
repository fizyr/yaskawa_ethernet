#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/commands.hpp"

#include "encode.hpp"
#include "decode.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

// write variable response
std::vector<std::uint8_t> encodeReadVariable(std::uint8_t request_id, int index, std::uint16_t command) {
	return encodeRequestHeader(makeRobotRequestHeader(0, command, index, 1, service::get_all, request_id));
}

ErrorOr<void> decodeWriteVariable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 0) return unexpectedValue("payload size", header->payload_size, 0);
	return dr::in_place_valid;
}

// byte variables

ErrorOr<std::uint8_t> decodeReadInt8Variable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 1) return unexpectedValue("payload size", header->payload_size, 1);
	return readLittleEndian<std::uint8_t>(message);
}

std::vector<std::uint8_t> encodeWriteInt8Variable(std::uint8_t request_id, int index, std::uint8_t value) {
	std::vector<std::uint8_t> result = encodeRequestHeader(makeRobotRequestHeader(1, commands::robot::readwrite_int8_variable, index, 1, service::set_all, request_id));
	writeLittleEndian<std::uint8_t>(result, value);
	return result;
}

// int (16 bit) variables

ErrorOr<std::int16_t> decodeReadInt16Variable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 2) return unexpectedValue("payload size", header->payload_size, 2);
	return readLittleEndian<std::int16_t>(message);
}

std::vector<std::uint8_t> encodeWriteInt16Variable(std::uint8_t request_id, int index, std::int16_t value) {
	std::vector<std::uint8_t> result = encodeRequestHeader(makeRobotRequestHeader(2, commands::robot::readwrite_int16_variable, index, 1, service::set_all, request_id));
	writeLittleEndian<std::int16_t>(result, value);
	return result;
}

// double int (32 bit) variables

ErrorOr<std::int32_t> decodeReadInt32Variable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 4) return unexpectedValue("payload size", header->payload_size, 4);
	return readLittleEndian<std::int32_t>(message);
}

std::vector<std::uint8_t> encodeWriteInt32Variable(std::uint8_t request_id, int index, std::int32_t value) {
	std::vector<std::uint8_t> result = encodeRequestHeader(makeRobotRequestHeader(4, commands::robot::readwrite_int32_variable, index, 1, service::set_all, request_id));
	writeLittleEndian<std::int32_t>(result, value);
	return result;
}

// real (32 bit float?) variables

ErrorOr<float> decodeReadFloat32Variable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 4) return unexpectedValue("payload size", header->payload_size, 4);
	std::uint32_t value = readLittleEndian<std::uint32_t>(message);
	return reinterpret_cast<float &>(value);
}

std::vector<std::uint8_t> encodeWriteFloat32Variable(std::uint8_t request_id, int index, float value) {
	std::vector<std::uint8_t> result = encodeRequestHeader(makeRobotRequestHeader(4, commands::robot::readwrite_float_variable, index, 1, service::set_all, request_id));
	writeLittleEndian<std::uint32_t>(result, reinterpret_cast<std::uint32_t &>(value));
	return result;
}

// position variables

ErrorOr<Position> decodeReadPositionVariable(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header) return header.error();
	if (header->payload_size != 13 * 4) return unexpectedValue("payload size", header->payload_size, 13 * 4);
	return decodePositionVariable(message);
}

std::vector<std::uint8_t> encodeWritePositionVariable(std::uint8_t request_id, int index, Position const & value, std::uint16_t command) {
	std::vector<std::uint8_t> result = encodeRequestHeader(makeRobotRequestHeader(13 * 4, command, index, 1, service::set_all, request_id));
	encodePosition(result, value);
	return result;
}

}}}
