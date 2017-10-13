#include "yaml.hpp"
#include <gtest/gtest.h>

int main(int argc, char ** argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

namespace dr {

TEST(Yaml, toYamlCartesianPosition) {
	yaskawa::CartesianPosition expected(0, 0, 0, 0, 2, 1);
	yaskawa::CartesianPosition returned = YAML::Node{expected}.as<yaskawa::CartesianPosition>();
	ASSERT_EQ(returned, expected);
}

TEST(Yaml, toYamlCoordinateSystem) {
	yaskawa::CoordinateSystem expected = yaskawa::CoordinateSystem::base;
	yaskawa::CoordinateSystem returned = YAML::Node{expected}.as<yaskawa::CoordinateSystem>();
	ASSERT_EQ(returned, expected);
}

}


