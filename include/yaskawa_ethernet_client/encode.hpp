#pragma once
#include <ostream>

namespace dr {
namespace yaskawa {

void writeStartRequest(std::ostream & out);

void writeStartRequest(std::ostream & out, int keep_alive);

void writeCommand(std::ostream & out, std::string const & command, std::size_t size);

}}
