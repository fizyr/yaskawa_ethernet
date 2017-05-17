#include "tcp/protocol.hpp"
#include "types.hpp"
#include "encode_parameters.hpp"

#include <sstream>
#include <iomanip>

namespace dr {
namespace yaskawa {

std::ostream & operator<<(std::ostream & stream, PulsePosition const & position) {
	for (int pulse : position.joints()) {
		stream << pulse << ",";
	}
	stream << position.tool();
	return stream;
}

std::ostream & operator<<(std::ostream & stream, CartesianPosition const & position) {
	stream << int(position.system) << ",";
	for (int i = 0; i < 3; ++i) {
		stream << std::setprecision(3) << std::fixed << position[i] << ",";
	}
	for (int i = 3; i < 6; ++i) {
		stream << std::setprecision(4) << std::fixed << position[i] << ",";
	}
	stream << int(position.type) << "," << int(position.tool);
	return stream;

}

std::ostream & operator<<(std::ostream & stream, Position const & position) {
	stream << int(position.type()) << ",";
	switch (position.type()) {
		case PositionType::pulse:     return stream << position.pulse();
		case PositionType::cartesian: return stream << position.cartesian();
	}
	throw std::logic_error("invalid position type");
}

namespace tcp {

namespace {

	void encodeCommand(boost::asio::streambuf & out, std::string const & command, std::size_t data_size) {
		std::ostream{&out} << "HOSTCTRL_REQUEST " << command << " " << data_size << "\r\n";
	}

	template<typename... T>
	void encodeCommandWithParams(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, std::string const & command, T const & ...params) {
		std::size_t start_size = params_out.size();
		encodeParameters(params_out, params...);
		encodeCommand(command_out, command, params_out.size() - start_size);
	}

	template<typename T>
	void encodeWriteVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, VariableType type, int index, T value) {
		encodeCommandWithParams(command_out, params_out, "LOADV", int(type), index, value);
	}
}

void encodeStartCommand(boost::asio::streambuf & command_out, int keep_alive) {
	if (keep_alive == 0) {
		std::ostream{&command_out} << "CONNECT Robot_access\r\n";
	} else {
		std::ostream{&command_out} << "CONNECT Robot_access Keep-Alive:" << keep_alive << "\r\n";
	}
}

void encodeServoOn(boost::asio::streambuf & command, boost::asio::streambuf & params, bool on) {
	encodeCommandWithParams(command, params, "SVON", int(on));
}

void encodeStartJob(boost::asio::streambuf & command, boost::asio::streambuf & params, std::string const & job) {
	encodeCommandWithParams(command, params, "START", job);
}

void encodeReadPulsePosition(boost::asio::streambuf & command_out, boost::asio::streambuf &) {
	encodeCommand(command_out, "RPOSJ", 0);
}

void encodeReadCartesianPosition(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, CoordinateSystem system) {
	encodeCommandWithParams(command_out, params_out, "RPOSC", int(system), 0);
}

void encodeReadIo(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int start, unsigned int count) {
	encodeCommandWithParams(command_out, params_out, "IOREAD", start, count);
}

void encodeWriteIo(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int start, std::vector<std::uint8_t> const & data) {
	std::ostream param_stream{&params_out};
	param_stream << start << "," << (data.size() * 8);
	for (auto const & byte : data) param_stream << "," << int(byte);
	encodeCommand(command_out, "IOWRITE", params_out.size());
}

void encodeReadVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, VariableType type, unsigned int index) {
	encodeCommandWithParams(command_out, params_out, "SAVEV", int(type), index);
}

void encodeReadByteVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index) {
	encodeReadVariable(command_out, params_out, VariableType::byte_type, index);
}

void encodeReadIntVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index) {
	encodeReadVariable(command_out, params_out, VariableType::integer_type, index);
}

void encodeReadDoubleIntVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index) {
	encodeReadVariable(command_out, params_out, VariableType::double_type, index);
}

void encodeReadRealVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index) {
	encodeReadVariable(command_out, params_out, VariableType::real_type, index);
}

void encodeReadPositionVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index) {
	encodeReadVariable(command_out, params_out, VariableType::robot_position_type, index);
}

void encodeWriteByteVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index, std::uint8_t value) {
	encodeWriteVariable(command_out, params_out, VariableType::byte_type, index, int(value));
}

void encodeWriteIntVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index, std::int16_t value) {
	encodeWriteVariable(command_out, params_out, VariableType::integer_type, index, value);
}

void encodeWriteDoubleIntVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index, std::int32_t value) {
	encodeWriteVariable(command_out, params_out, VariableType::double_type, index, value);
}

void encodeWriteRealVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index, float value) {
	encodeWriteVariable(command_out, params_out, VariableType::real_type, index, value);
}

void encodeWritePositionVariable(boost::asio::streambuf & command_out, boost::asio::streambuf & params_out, unsigned int index, Position const & value) {
	encodeWriteVariable(command_out, params_out, VariableType::robot_position_type, index, value);
}

}}}
