#ifndef __MCD_CORE_MATH_RAY__
#define __MCD_CORE_MATH_RAY__

#include "Vec3.h"

namespace MCD {

/*!	Representation of a ray in space.
	A line with an origin and an UNIT direction.
 */
class Ray
{
public:
	Ray() {}

	Ray(const Vec3f& _origin, const Vec3f& _unitDirection)
		: origin(_origin), unitDirection(_unitDirection)
	{}

// Attribute
	Vec3f origin;
	Vec3f unitDirection;
};	// Ray

}	// namespace MCD

#endif	// __MCD_CORE_MATH_RAY__
