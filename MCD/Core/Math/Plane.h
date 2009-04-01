#ifndef __MCD_CORE_MATH_PLANE__
#define __MCD_CORE_MATH_PLANE__

#include "Vec3.h"

namespace MCD {

/*!	Defines a plane in 3D space.
	A plane is defined in 3D space by the equation Ax + By + Cz + D = 0
	This equates to a vector (the normal of the plane, whose x, y and z components
	equate to the coefficients A, B and C respectively), and a constant (D) which
	is the distance along the normal you have to go to move the plane back to the origin.
 */
class Plane
{
public:
	Plane() {}

	Plane(const Vec3f& _normal, float _constant)
		: normal(_normal), constant(_constant)
	{}

	Plane(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3)
	{
		MCD_ASSERT(false && "To be implement");
	}

// Attribute
	Vec3f normal;
	float constant;
};	// Plane

}	// namespace MCD

#endif	// __MCD_CORE_MATH_PLANE__
