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
