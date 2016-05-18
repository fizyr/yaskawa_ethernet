#include "udp/protocol.hpp"
#include "udp/message.hpp"
#include "udp/commands.hpp"

namespace dr {
namespace yaskawa {
namespace udp {

namespace {
	template<typename T>
	void writeLittleEndian(std::vector<std::uint8_t> & out, T value) {
		static_assert(std::is_integral<T>::value, "T must be an integral type.");
		for (unsigned int i = 0; i < sizeof(T); ++i) {
			out.push_back(value >> i * 8 & 0xff);
		}
	}

	template<typename T>
	T readLittleEndian(std::uint8_t const * data) {
		static_assert(std::is_integral<T>::value, "T must be an integral type.");
		T result = 0;
		for (unsigned int i = 0; i < sizeof(T); ++i) {
			result |= data[i] << i * 8;
		}
		return result;
	}

	template<typename T>
	T readLittleEndian(string_view & data) {
		data.remove_prefix(sizeof(T));
		return readLittleEndian<T>(reinterpret_cast<std::uint8_t const *>(data.data() - sizeof(T)));
	}

	void encodeRequestHeader(
		std::vector<std::uint8_t> & out,
		RequestHeader const & header
	) {
		out.reserve(out.size() + header_size + header.payload_size);
		// Magic bytes.
		out.insert(out.end(), {'Y', 'E', 'R', 'C'});

		// Header size, payload size.
		writeLittleEndian<std::uint16_t>(out, header_size);
		writeLittleEndian<std::uint16_t>(out, header.payload_size);

		// Reserved magic constant.
		out.push_back('3');

		// "Division" (robot command or file command)
		out.push_back(std::uint8_t(header.division));

		// Ack (should always be zero for requests).
		out.push_back(header.ack);

		// Request ID.
		out.push_back(header.request_id);

		// Block number.
		writeLittleEndian<std::uint32_t>(out, header.block_number);

		// Reserved.
		out.insert(out.end(), 8, '9');

		// Subrequest details
		writeLittleEndian<std::uint16_t>(out, header.command);
		writeLittleEndian<std::uint16_t>(out, header.instance);
		out.push_back(header.attribute);
		out.push_back(header.service);

		// Padding.
		out.insert(out.end(), 2, 0);
	}

	RequestHeader makeRobotRequestHeader(
		std::uint16_t payload_size,
		std::uint16_t command,
		std::uint16_t instance,
		std::uint8_t attribute,
		std::uint8_t service,
		std::uint8_t request_id = 0
	) {
		RequestHeader header;
		header.payload_size = payload_size;
		header.division     = Division::robot;
		header.ack          = false;
		header.request_id   = request_id;
		header.block_number = 0;
		header.command      = command;
		header.instance     = instance;
		header.attribute    = attribute;
		header.service      = service;
		return header;
	}

	DetailedError malformedResponse(std::string && message) {
		return {boost::system::error_code{errc::malformed_response}, std::move(message)};
	}

	DetailedError maximumExceeded(std::string && field, int value, int max) {
		return malformedResponse(
			"received " + field + " (" + std::to_string(value) + ") "
			"exceeds maximum value (" + std::to_string(max) + ")"
		);
	}

	DetailedError unexpectedValue(std::string && field, int value, int expected) {
		return malformedResponse(
			"received " + field + " (" + std::to_string(value) + ") "
			"does not match the expected value (" + std::to_string(expected) + ")"
		);
	}

	DetailedError commandFailed(int status, int extra_status) {
		return {boost::system::error_code{errc::command_failed},
			"command failed with status " + std::to_string(status)
			+ " and additional status " + std::to_string(extra_status)
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
		if (result.payload_size > max_payaload_size) return maximumExceeded("payload size", result.payload_size, max_payaload_size);
		if (original.size() != header_size + result.payload_size) return malformedResponse(
			"number of received bytes (" + std::to_string(original.size()) + ") "
			"does not match the message size according to the header "
			"(" + std::to_string(header_size + result.payload_size) + ")"
		);

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

		// Parse extra status field. Holy shit.
		std::uint8_t additional_status_size = readLittleEndian<std::uint8_t>(data);
		data.remove_prefix(1);
		result.extra_status = readLittleEndian<std::uint16_t>(data);

		// Padding.
		data.remove_prefix(2);

		if (result.status != 0) return commandFailed(result.status, result.extra_status);
		return result;
	}
}

template<> std::vector<std::uint8_t> encode<ReadInt8Variable::Request>(ReadInt8Variable::Request const & request, std::uint8_t request_id) {
	std::vector<std::uint8_t> result;
	RequestHeader header = makeRobotRequestHeader(0, commands::robot::read_int8_variable, request.index, 1, service::get_all, request_id);
	encodeRequestHeader(result, header);
	return result;
}

template<> ErrorOr<ReadInt8Variable::Response> decode<ReadInt8Variable::Response>(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header.valid()) return header.error();
	if (header.get().payload_size != 4) return unexpectedValue("payload size", header.get().payload_size, 4);
	return ReadInt8Variable::Response{readLittleEndian<std::uint8_t>(message)};
}

template<> std::vector<std::uint8_t> encode<WriteInt8Variable::Request>(WriteInt8Variable::Request const & request, std::uint8_t request_id) {
	std::vector<std::uint8_t> result;
	RequestHeader header = makeRobotRequestHeader(4, commands::robot::read_int8_variable, request.index, 1, service::set_single, request_id);
	encodeRequestHeader(result, header);
	result.push_back(request.value);
	result.insert(result.end(), 3, 0);
	return result;
}

template<> ErrorOr<WriteInt8Variable::Response> decode<WriteInt8Variable::Response>(string_view message) {
	ErrorOr<ResponseHeader> header = decodeResponseHeader(message);
	if (!header.valid()) return header.error();
	if (header.get().payload_size != 0) return unexpectedValue("payload size", header.get().payload_size, 0);
	return ErrorOr<void>{};
}

}}}
