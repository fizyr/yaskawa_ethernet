/* Copyright 2016-2019 Fizyr B.V. - https://fizyr.com
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "yaml.hpp"
#include <yaml-cpp/yaml.h>

YAML::Node YAML::convert<dr::yaskawa::CartesianPosition>::encode(dr::yaskawa::CartesianPosition const & in) {
	YAML::Node node;
	node["x"] = in.x();
	node["y"] = in.y();
	node["z"] = in.z();
	node["rx"] = in.rx();
	node["ry"] = in.ry();
	node["rz"] = in.rz();
	node["frame"] = in.frame();
	node["configuration"] = int(in.configuration());
	node["tool"] = in.tool();
	return node;
}

bool YAML::convert<dr::yaskawa::CartesianPosition>::decode(Node const & node, dr::yaskawa::CartesianPosition & out) {
	if (!node.IsMap() || node.size() != 9) return false;
	out = {
		node["x"].as<double>(),
		node["y"].as<double>(),
		node["z"].as<double>(),
		node["rx"].as<double>(),
		node["ry"].as<double>(),
		node["rz"].as<double>(),
		node["frame"].as<dr::yaskawa::CoordinateSystem>(),
		std::uint8_t(node["configuration"].as<int>()),
		node["tool"].as<int>()
	};
	return true;
}

YAML::Node YAML::convert<dr::yaskawa::CoordinateSystem>::encode(dr::yaskawa::CoordinateSystem const & in) {
	YAML::Node node = YAML::Node{dr::yaskawa::toString(in)};
	return node;
}

bool YAML::convert<dr::yaskawa::CoordinateSystem>::decode(Node const & node, dr::yaskawa::CoordinateSystem & out) {
	out = dr::yaskawa::toCoordinateSystem(node.as<std::string>()).value();
	return true;
}
