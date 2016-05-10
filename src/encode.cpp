#include "encode.hpp"

namespace dr {
namespace yaskawa {

void writeStartRequest(std::ostream & out) {
	out << "CONNECT Robot_access\r\n";
}

void writeStartRequest(std::ostream & out, int keep_alive) {
	out << "CONNECT Robot_access Keep-Alive:" << keep_alive << "\r\n";
}

void writeCommand(std::ostream & out, std::string const & command, std::size_t size) {
	out << "HOSTCTRL_REQUEST " << command << " " << size << "\r\n";
}

}}
