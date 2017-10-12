#include "types.hpp"
#include <yaml-cpp/yaml.h>
#include <string>

namespace YAML {
	template<>
	struct convert<dr::yaskawa::CartesianPosition>{
		static Node encode(dr::yaskawa::CartesianPosition const & in);
		static bool decode(Node const & node, dr::yaskawa::CartesianPosition & out);
	};

	template<>
	struct convert<dr::yaskawa::CoordinateSystem>{
		static Node encode(dr::yaskawa::CoordinateSystem const & in);
		static bool decode(Node const & node, dr::yaskawa::CoordinateSystem & out);
	};
}
