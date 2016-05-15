#pragma once
#include "messages.hpp"

#include <ostream>

namespace dr {
namespace yaskawa {

/// Encode a START request.
void encodeStartRequest(std::ostream & out, int keep_alive);
inline void encodeStartRequest(std::ostream && out, int keep_alive) { encodeStartRequest(out, keep_alive); }

/// Encode a command request.
void encodeCommand(std::ostream & out, std::string const & command, std::size_t size);
inline void encodeCommand(std::ostream && out, std::string const & command, std::size_t size) { encodeCommand(out, command, size); }

/// Encode a read variable request.
void encodeReadVariable(std::ostream & out, VariableType type, int index);
inline void encodeReadVariable(std::ostream && out, VariableType type, int index) { encodeReadVariable(out, type, index); }

/// Encode a read variable request.
void encodeWriteByteVariable(std::ostream & out, int index, std::uint8_t value);
inline void encodeWriteByteVariable(std::ostream && out, int index, std::uint8_t value) { encodeWriteByteVariable(out, index, value); }

}}
