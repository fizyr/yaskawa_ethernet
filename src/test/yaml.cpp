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


