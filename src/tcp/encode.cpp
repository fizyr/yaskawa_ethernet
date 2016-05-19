#include "tcp/protocol.hpp"
#include "types.hpp"
#include "encode_parameters.hpp"

#include <sstream>

namespace dr {
namespace yaskawa {
namespace tcp {

namespace {

	void encodeCommand(std::ostream & out, std::string const & command, std::size_t data_size) {
		out << "HOSTCTRL_REQUEST " << command << " " << data_size << "\r\n";
	}

	template<typename... T>
	void encodeCommandWithParams(std::ostream & out, std::string const & command, T const & ...params) {
		std::stringstream buffer;
		encodeParameters(buffer, params...);
		encodeCommand(out, command, buffer.tellp());
		out << buffer.rdbuf();
	}

	void encodeReadVariable(std::ostream & out, VariableType type, int index) {
		encodeCommandWithParams(out, "SAVEV", int(type), index);
	}

	template<typename T>
	void encodeWriteVariable(std::ostream & out, VariableType type, int index, T value) {
		encodeCommandWithParams(out, "LOADV", int(type), index, value);
	}
}

template<>
void encode<StartCommand::Request>(std::ostream & out, StartCommand::Request request) {
	if (request.keep_alive == 0) {
		out << "CONNECT Robot_access\r\n";
	} else {
		out << "CONNECT Robot_access Keep-Alive:" << request.keep_alive << "\r\n";
	}
}

template<>
void encode<ReadInt8Variable::Request> (std::ostream & out, ReadInt8Variable::Request request) {
	encodeReadVariable(out, VariableType::byte_type, request);
}

template<>
void encode<ReadInt16Variable::Request> (std::ostream & out, ReadInt16Variable::Request request) {
	encodeReadVariable(out, VariableType::integer_type, request);
}

template<>
void encode<ReadInt32Variable::Request> (std::ostream & out, ReadInt32Variable::Request request) {
	encodeReadVariable(out, VariableType::double_type, request);
}

template<>
void encode<ReadFloat32Variable::Request>(std::ostream & out, ReadFloat32Variable::Request request) {
	encodeReadVariable(out, VariableType::real_type, request);
}

template<>
void encode<WriteInt8Variable::Request> (std::ostream & out, WriteInt8Variable::Request request) {
	encodeWriteVariable(out, VariableType::byte_type, request.index, request.value);
}

template<>
void encode<WriteInt16Variable::Request> (std::ostream & out, WriteInt16Variable::Request request) {
	encodeWriteVariable(out, VariableType::integer_type, request.index, request.value);
}

template<>
void encode<WriteInt32Variable::Request> (std::ostream & out, WriteInt32Variable::Request request) {
	encodeWriteVariable(out, VariableType::double_type, request.index, request.value);
}

template<>
void encode<WriteFloat32Variable::Request> (std::ostream & out, WriteFloat32Variable::Request request) {
	encodeWriteVariable(out, VariableType::real_type, request.index, request.value);
}

}}}
