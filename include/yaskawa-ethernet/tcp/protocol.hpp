#pragma once
#include "impl/response_matcher.hpp"
#include "../commands.hpp"
#include "../error.hpp"
#include "../string_view.hpp"

#include <boost/asio/streambuf.hpp>

namespace dr {
namespace yaskawa {
namespace tcp {

struct CommandResponse {
	std::string message;

	CommandResponse() = default;
	CommandResponse(std::string const & message) : message(message) {};
	CommandResponse(std::string      && message) : message(std::move(message)) {};

	operator std::string const & () const & { return message; }
	operator std::string       & ()       & { return message; }
	operator std::string      && ()      && { return std::move(message); }
};

struct StartCommand {
	struct Request  { int keep_alive; };
	using Response = CommandResponse;
};

/// Default constructible function object type that matches response messages.
/**
 * For use with boost::asio::read_until.
 */
using ResponseMatcher = impl::ResponseMatcher;

void encodeStartCommand(boost::asio::streambuf & command, int keep_alive);

void encodeReadPulsePosition(boost::asio::streambuf & command, boost::asio::streambuf & params);
void encodeReadCartesianPosition(boost::asio::streambuf & command, boost::asio::streambuf & params, CoordinateSystem system);

void encodeReadVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, VariableType type, unsigned int index);
void encodeReadByteVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index);
void encodeReadIntVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index);
void encodeReadDoubleIntVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index);
void encodeReadRealVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index);
void encodeReadPositionVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index);

void encodeWriteByteVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index, std::uint8_t value);
void encodeWriteIntVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index, std::int16_t value);
void encodeWriteDoubleIntVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index, std::int32_t value);
void encodeWriteRealVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index, float value);
void encodeWritePositionVariable(boost::asio::streambuf & command, boost::asio::streambuf & params, unsigned int index, Position const & position);

ErrorOr<CommandResponse> decodeCommandResponse(string_view);
ErrorOr<void> decodeEmptyData(string_view);

ErrorOr<PulsePosition> decodeReadPulsePosition(string_view view);
ErrorOr<CartesianPosition> decodeReadCartesianPosition(string_view view);

ErrorOr<std::uint8_t> decodeReadByteVariable(string_view);
ErrorOr<std::int16_t> decodeReadIntVariable(string_view);
ErrorOr<std::int32_t> decodeReadDoubleIntVariable(string_view);
ErrorOr<float> decodeReadRealVariable(string_view);
ErrorOr<Position> decodeReadPositionVariable(string_view);

}}}
