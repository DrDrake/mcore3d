#ifndef __MCD_CORE_MATH_INTERSECTION__
#define __MCD_CORE_MATH_INTERSECTION__

#include "Vec3.h"
#include "../ShareLib.h"

namespace MCD {

class Plane;
class Ray;

/*!	Tests whether a ray intersects with a plane.
	\param distanceAlongRay
		indicate the distance along the ray at which it intersects.
		return std::numeric_limits<float>::max() if there is no intersection.
 */
sal_checkreturn bool MCD_CORE_API Intersects(const Ray& ray, const Plane& plane, float& distanceAlongRay);

}	// namespace MCD

#endif	// __MCD_CORE_MATH_INTERSECTION__
