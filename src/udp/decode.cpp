#include "decode.hpp"
#include "udp/protocol.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

char hexNibble(int value) {
	if (value < 10) return '0' + value;
	return 'A' + value - 10;
}

template<typename T>
std::string toHex(T val) {
	static_assert(std::is_integral<T>::value, "");
	std::string result(sizeof(T) * 2, '0');
	for (unsigned int i = 0; i < sizeof(T); i++) {
		result[(sizeof(T) - i) * 2 - 1] = hexNibble(val >> (8 * i)     & 0x0f);
		result[(sizeof(T) - i) * 2 - 2] = hexNibble(val >> (8 * i + 4) & 0x0f);
	}
	return result;
}

DetailedError malformedResponse(std::string message) {
	return {errc::malformed_response, std::move(message)};
}

DetailedError maximumExceeded(std::string field, int value, int max) {
	return malformedResponse(
		"received " + std::move(field) + " (" + std::to_string(value) + ") "
		"exceeds maximum value (" + std::to_string(max) + ")"
	);
}

DetailedError unexpectedValue(std::string field, int value, int expected) {
	return malformedResponse(
		"received " + std::move(field) + " (" + std::to_string(value) + ") "
		"does not match the expected value (" + std::to_string(expected) + ")"
	);
}

DetailedError commandFailed(std::uint16_t status, std::uint16_t extra_status) {
	return {errc::command_failed,
		"command failed with status 0x" + toHex(status)
		+ " and additional status 0x" + toHex(extra_status)
	};
}

ErrorOr<ResponseHeader> decodeResponseHeader(string_view & data) {
	string_view original = data;
	ResponseHeader result;

	// Make sure we can parse the header safely.
	if (data.size() < header_size) return malformedResponse(
		"response (" + std::to_string(data.size()) + " bytes) "
		"does not contain enough data for a header "
		"(" + std::to_string(header_size) + " bytes)"
	);

	// Check the magic bytes.
	if (data.substr(0, 4) != "YERC") return malformedResponse("response does not start with magic bytes `YERC'");
	data.remove_prefix(4);

	// Check the header size.
	std::uint16_t parsed_header_size = readLittleEndian<std::uint16_t>(data);
	if (parsed_header_size != header_size) return unexpectedValue("header size", parsed_header_size, header_size);

	// Get payload size and make sure the message is complete.
	result.payload_size = readLittleEndian<std::uint16_t>(data);
	if (result.payload_size > max_payload_size) return maximumExceeded("payload size", result.payload_size, max_payload_size);

	data.remove_prefix(1);
	result.division = Division(readLittleEndian<std::uint8_t>(data));

	// Make sure the ack value is correct.
	std::uint8_t ack = readLittleEndian<std::uint8_t>(data);
	if (ack != 1) return unexpectedValue("ACK value", ack, 1);
	result.ack = true;

	// Parse request ID and block number.
	result.request_id   = readLittleEndian<std::uint8_t>(data);
	result.block_number = readLittleEndian<std::uint32_t>(data);

	// Reserved 8 bytes.
	data.remove_prefix(8);

	// Parse service and status field.
	result.service = readLittleEndian<std::uint8_t>(data);
	result.status  = readLittleEndian<std::uint8_t>(data);

	// Ignore added status size, just treat it as two byte value.
	data.remove_prefix(2);
	result.extra_status = readLittleEndian<std::uint16_t>(data);

	// Padding.
	data.remove_prefix(2);

	if (original.size() != header_size + result.payload_size) return malformedResponse(
		"request " + std::to_string(int(result.request_id)) + ": "
		"number of received bytes (" + std::to_string(original.size()) + ") "
		"does not match the message size according to the header "
		"(" + std::to_string(header_size + result.payload_size) + ")"
	);

	return result;
}

ErrorOr<CoordinateSystem> decodeCartesianFrame(int type, int user_frame) {
	switch (type) {
		case 16: return CoordinateSystem::base;
		case 17: return CoordinateSystem::robot;
		case 18: return CoordinateSystem::tool;
		case 19:
			if (user_frame > 15) {
				return maximumExceeded("user frame", user_frame, 15);
			}
			return userCoordinateSystem(user_frame);
	}
	return malformedResponse("unknown position type (" + std::to_string(type) + "), expected 16, 17, 18 or 19");
}

}}}
