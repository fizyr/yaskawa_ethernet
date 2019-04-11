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

#include "types.hpp"
#include "eigen.hpp"

#include <cmath>

namespace dr {
namespace yaskawa {

/// Convert a cartesian robot position to an isometry.
/**
 * The resulting isometry has the translation part specified in meters.
 */
Eigen::Isometry3d toEigen(CartesianPosition const & position) {
	return Eigen::Translation3d{Eigen::Vector3d{position.x(), position.y(), position.z()} * 0.001}
		* Eigen::AngleAxisd{position.rz() * M_PI / 180.0, Eigen::Vector3d::UnitZ()}
		* Eigen::AngleAxisd{position.ry() * M_PI / 180.0, Eigen::Vector3d::UnitY()}
		* Eigen::AngleAxisd{position.rx() * M_PI / 180.0, Eigen::Vector3d::UnitX()}
		;
}

CartesianPosition toCartesian(Eigen::Isometry3d const & pose, CoordinateSystem frame, int tool, PoseConfiguration configuration) {
	CartesianPosition result;
	result.frame()         = frame;
	result.tool()          = tool;
	result.configuration() = configuration;

	Eigen::Vector3d angles = pose.rotation().eulerAngles(2, 1, 0);
	result.x() = pose.translation().x() * 1000;
	result.y() = pose.translation().y() * 1000;
	result.z() = pose.translation().z() * 1000;
	result.rx() = angles[2] / M_PI * 180.0;
	result.ry() = angles[1] / M_PI * 180.0;
	result.rz() = angles[0] / M_PI * 180.0;
	return result;
}

}}
