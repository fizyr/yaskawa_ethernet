#pragma once
#include "messages.hpp"
#include <ostream>

namespace dr {
namespace yaskawa {

void encodeStartRequest(std::ostream & out, int keep_alive);
inline void encodeStartRequest(std::ostream && out, int keep_alive) { encodeStartRequest(out, keep_alive); }

void encodeCommand(std::ostream & out, std::string const & command, std::size_t size);
inline void encodeCommand(std::ostream && out, std::string const & command, std::size_t size) { encodeCommand(out, command, size); }

void encodeReadVariable(std::ostream & out, VariableType type, int number);
inline void encodeReadVariable(std::ostream && out, VariableType type, int number) { encodeReadVariable(out, type, number); }

}}
