#pragma once
#include "impl/response_matcher.hpp"
#include "../types.hpp"
#include "../error.hpp"
#include "../string_view.hpp"

#include <dr_error/error_or.hpp>

#include <asio/streambuf.hpp>

#include <vector>
#include <cstdint>

namespace dr {
namespace yaskawa {
namespace tcp {

/// Default constructible function object type that matches response messages.
/**
 * For use with asio::read_until.
 */
using ResponseMatcher = impl::ResponseMatcher;

void encodeStartCommand(asio::streambuf & command, int keep_alive);

void encodeServoOn(asio::streambuf & command, asio::streambuf & params, bool on);
void encodeStartJob(asio::streambuf & command, asio::streambuf & params, std::string const & name);

void encodeReadPulsePosition(asio::streambuf & command, asio::streambuf & params);
void encodeReadCartesianPosition(asio::streambuf & command, asio::streambuf & params, CoordinateSystem system);

void encodeReadIo(asio::streambuf & command, asio::streambuf & params, unsigned int start, unsigned int count);
void encodeWriteIo(asio::streambuf & command, asio::streambuf & params, unsigned int start, std::vector<std::uint8_t> const & data);

void encodeReadVariable(asio::streambuf & command, asio::streambuf & params, VariableType type, unsigned int index);
void encodeReadByteVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index);
void encodeReadIntVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index);
void encodeReadDoubleIntVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index);
void encodeReadRealVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index);
void encodeReadPositionVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index);

void encodeWriteByteVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index, std::uint8_t value);
void encodeWriteIntVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index, std::int16_t value);
void encodeWriteDoubleIntVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index, std::int32_t value);
void encodeWriteRealVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index, float value);
void encodeWritePositionVariable(asio::streambuf & command, asio::streambuf & params, unsigned int index, Position const & position);

ErrorOr<std::string> decodeCommandResponse(string_view);
ErrorOr<void> decodeEmptyData(string_view);

ErrorOr<PulsePosition> decodeReadPulsePosition(string_view view);
ErrorOr<CartesianPosition> decodeReadCartesianPosition(string_view view);
ErrorOr<std::vector<std::uint8_t>> decodeReadIo(string_view);

ErrorOr<std::uint8_t> decodeReadByteVariable(string_view);
ErrorOr<std::int16_t> decodeReadIntVariable(string_view);
ErrorOr<std::int32_t> decodeReadDoubleIntVariable(string_view);
ErrorOr<float> decodeReadRealVariable(string_view);
ErrorOr<Position> decodeReadPositionVariable(string_view);

}}}
