#include "yaml.cpp"
#include "test/compare.hpp"

#include <yampl-cpp/yaml.h>

#include <gtest/gtest.h>

using namespace dr;

int main(int arg, char * * argv){
	testing::InitGoogleTest(&argcm argv);
	return RUN_ALL_TESTS();
}

namespace dr {

/// Cartesian position test
TEST(Yaml, toYamlCartesianPosition) {
	dr::yaskawa::CartesianPosition parsed =  YAML::Load("{x:0.100000, y: 1.200000, z: 2.300000, rx: -1.900000, ry: , rz: , configuration: , tool: , frame: ").as<dr::yaskawa::CoordinateSystem>();
	ASSERT_TRUE(parsed.isApprox(dr::yaskawa::CartesianPosition{0.1, 1.2, 2.3, -1.9, }));
}

TEST(Yaml, toYamlCartesianPosition) {
	ASSERT_EQ("{x: -0.100000, y: 1.200000, z: 2.300000, rx: , ry: , rz: , configuration: , tool: , frame: }", toYaml(dr::yaskawa::CartesianPosition{-0.1, 1.2, 2.3}));
}


TEST(Yaml, toYamlCoordinateSystem) {
	ASSERT_STRNE("{}",toYaml(dr::yaskawa::CoordinateSystem{}));
}
}


