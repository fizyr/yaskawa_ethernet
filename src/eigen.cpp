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
		* Eigen::AngleAxisd{position.rx() * M_PI / 180.0, Eigen::Vector3d::UnitX()}
		* Eigen::AngleAxisd{position.ry() * M_PI / 180.0, Eigen::Vector3d::UnitY()}
		* Eigen::AngleAxisd{position.rz() * M_PI / 180.0, Eigen::Vector3d::UnitZ()};
}

CartesianPosition toCartesian(Eigen::Isometry3d const & pose, CoordinateSystem system, int tool, CartesianPoseType type) {
	CartesianPosition result;
	result.system = system;
	result.tool   = tool;
	result.type   = type;

	Eigen::Vector3d angles = pose.rotation().eulerAngles(0, 1, 2);
	result.x() = pose.translation().x() * 1000;
	result.y() = pose.translation().y() * 1000;
	result.z() = pose.translation().z() * 1000;
	result.rx() = angles[0] / M_PI * 180.0;
	result.ry() = angles[1] / M_PI * 180.0;
	result.rz() = angles[2] / M_PI * 180.0;
	return result;
}

}}
