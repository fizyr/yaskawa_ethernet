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
	void encodeCommandWithParams(std::ostream & command_out, std::ostream & params_out, std::string const & command, T const & ...params) {
		std::size_t start_size = params_out.tellp();
		encodeParameters(params_out, params...);
		encodeCommand(command_out, command, std::size_t(params_out.tellp()) - start_size);
	}

	void encodeReadVariable(std::ostream & command_out, std::ostream & params_out, VariableType type, int index) {
		encodeCommandWithParams(command_out, params_out, "SAVEV", int(type), index);
	}

	template<typename T>
	void encodeWriteVariable(std::ostream & command_out, std::ostream & params_out, VariableType type, int index, T value) {
		encodeCommandWithParams(command_out, params_out, "LOADV", int(type), index, value);
	}
}

template<>
void encode<StartCommand::Request>(std::ostream & command_out, std::ostream & params_out, StartCommand::Request request) {
	(void) params_out;
	if (request.keep_alive == 0) {
		command_out << "CONNECT Robot_access\r\n";
	} else {
		command_out << "CONNECT Robot_access Keep-Alive:" << request.keep_alive << "\r\n";
	}
}

template<>
void encode<ReadInt8Variable::Request> (std::ostream & command_out, std::ostream & params_out, ReadInt8Variable::Request request) {
	encodeReadVariable(command_out, params_out, VariableType::byte_type, request);
}

template<>
void encode<ReadInt16Variable::Request> (std::ostream & command_out, std::ostream & params_out, ReadInt16Variable::Request request) {
	encodeReadVariable(command_out, params_out, VariableType::integer_type, request);
}

template<>
void encode<ReadInt32Variable::Request> (std::ostream & command_out, std::ostream & params_out, ReadInt32Variable::Request request) {
	encodeReadVariable(command_out, params_out, VariableType::double_type, request);
}

template<>
void encode<ReadFloat32Variable::Request>(std::ostream & command_out, std::ostream & params_out, ReadFloat32Variable::Request request) {
	encodeReadVariable(command_out, params_out, VariableType::real_type, request);
}

template<>
void encode<WriteInt8Variable::Request> (std::ostream & command_out, std::ostream & params_out, WriteInt8Variable::Request request) {
	encodeWriteVariable(command_out, params_out, VariableType::byte_type, request.index, request.value);
}

template<>
void encode<WriteInt16Variable::Request> (std::ostream & command_out, std::ostream & params_out, WriteInt16Variable::Request request) {
	encodeWriteVariable(command_out, params_out, VariableType::integer_type, request.index, request.value);
}

template<>
void encode<WriteInt32Variable::Request> (std::ostream & command_out, std::ostream & params_out, WriteInt32Variable::Request request) {
	encodeWriteVariable(command_out, params_out, VariableType::double_type, request.index, request.value);
}

template<>
void encode<WriteFloat32Variable::Request> (std::ostream & command_out, std::ostream & params_out, WriteFloat32Variable::Request request) {
	encodeWriteVariable(command_out, params_out, VariableType::real_type, request.index, request.value);
}

}}}
