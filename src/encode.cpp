#include "encode.hpp"
#include "encode_parameters.hpp"

#include <sstream>

namespace dr {
namespace yaskawa {

void encodeStartRequest(std::ostream & out, int keep_alive) {
	if (keep_alive == 0) {
		out << "CONNECT Robot_access\r\n";
	} else {
		out << "CONNECT Robot_access Keep-Alive:" << keep_alive << "\r\n";
	}
}

void encodeCommand(std::ostream & out, std::string const & command, std::size_t data_size) {
	out << "HOSTCTRL_REQUEST " << command << " " << data_size << "\r\n";
}

namespace {
	template<typename... T>
	void encodeCommandWithParams(std::ostream & out, std::string const & command, T const & ...params) {
		std::stringstream buffer;
		encodeParameters(buffer, params...);
		encodeCommand(out, command, buffer.tellp());
		out << buffer.rdbuf();
	}
}

void encodeReadVariable(std::ostream & out, VariableType type, int index) {
	encodeCommandWithParams(out, "SAVEV", int(type), index);
}

}}
