#include "yaml.hpp"
#include <gtest/gtest.h>

int main(int argc, char ** argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

namespace dr {


TEST(Yaml, toYamlCartesianPosition) {
	yaskawa::CartesianPosition expected(0.100000, -2.555555, 4.8787567890, -1, 0, -32, yaskawa::CoordinateSystem(7), 2, 1);
	yaskawa::CartesianPosition returned = YAML::Node{expected}.as<yaskawa::CartesianPosition>();
	ASSERT_EQ(returned, expected);
}

TEST(Yaml, toYamlCoordinateSystem) {
	yaskawa::CoordinateSystem expected = yaskawa::CoordinateSystem::base;
	yaskawa::CoordinateSystem returned = YAML::Node{expected}.as<yaskawa::CoordinateSystem>();
	ASSERT_EQ(returned, expected);
}

TEST(Yaml, parsingIsCorrect) {
	yaskawa::CartesianPosition expected(-1.22434359, 2.3456785, -5.6534534, 0, 3.44522, 2.22222, yaskawa::CoordinateSystem(0), 4, 1);
	YAML::Node returned = YAML::Load("{x: -1.22434359, y: 2.3456785, z: -5.6534534, rx: 0, ry: 3.44522, rz: 2.22222, frame: base , configuration: 4, tool: 1}");
	yaskawa::CartesianPosition actual = returned.as<yaskawa::CartesianPosition>();
	ASSERT_EQ(expected.x(), actual.x());
	ASSERT_EQ(expected.y(), actual.y());
	ASSERT_EQ(expected.z(), actual.z());
	ASSERT_EQ(expected.rx(), actual.rx());
	ASSERT_EQ(expected.ry(), actual.ry());
	ASSERT_EQ(expected.rz(), actual.rz());
	ASSERT_EQ(expected.frame(), actual.frame());
	ASSERT_EQ(expected.tool(), actual.tool());
	ASSERT_EQ(expected.configuration(), actual.configuration());
}

}


