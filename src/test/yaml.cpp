#include "yaml.hpp"
#include <gtest/gtest.h>

using namespace dr;

int main(int argc, char ** argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

namespace dr {

TEST(Yaml, toYamlCartesianPosition) {
	ASSERT_EQ(5, 5);
}

TEST(Yaml, toYamlCoordinateSystem) {
	ASSERT_EQ(0, 0);
}

}


