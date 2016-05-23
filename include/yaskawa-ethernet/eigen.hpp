#include "types.hpp"

#include <Eigen/Geometry>

namespace dr {
namespace yaskawa {

/// Convert a cartesian robot position to an isometry.
/**
 * The resulting isometry has the translation part specified in meters.
 */
Eigen::Isometry3d toEigen(CartesianPosition const & position);

CartesianPosition toCartesian(Eigen::Isometry3d const & pose, CoordinateSystem system = CoordinateSystem::base, int tool = 0, CartesianPoseType type = {});

}}
