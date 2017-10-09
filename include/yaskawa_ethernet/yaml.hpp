#include "types.hpp"

#include <yaml-cpp/yaml.h>

#include<string>

namespace dr{

/// Convert Cartesian position to YAML.
YAML::convert<dr::yaskawa::CartesianPosition>::decode(Node & node, CartesianPosition const & out);

}

namespace YAML {
	/// Convert between yamlcpp and CartesianPosition.
	template<>
	struct convert<dr::yaskawa::CartesianPosition>{
		static Node encode(dr::yaskawa::CartesianPosition const & in);
		static bool decode(Node const & node, dr::yaskawa::CartesianPosition & out);
	};



}
