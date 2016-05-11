#pragma once
#include "messages.hpp"
#include <ostream>

namespace dr {
namespace yaskawa {

void encodeStartRequest(std::ostream & out, int keep_alive);

void encodeCommand(std::ostream & out, std::string const & command, std::size_t size);

void encodeReadVariableRequest(std::ostream & out, VariableType type, int number);

}}
