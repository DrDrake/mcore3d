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
// Construction
	Plane() {}

	Plane(const Vec3f& _normal, float _d)
		: normal(_normal), d(_d)
	{}

	Plane(float _a, float _b, float _c, float _d)
		: normal(_a, _b, _c), d(_d)
	{}

	//! Define the plane using a point and a normal.
	Plane(const Vec3f& _normal, const Vec3f& point)
		: normal(_normal), d(-_normal.dot(point))
	{}

	/*! Define the plane using 3 points.
		The winding order will defind the direction of the normal
	 */
	Plane(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
	{
		normal = (p1 - p2) ^ (p2 - p3);
		d = -normal.dot(p1);
	}

// Attribute
	Vec3f normal;
	float d;
};	// Plane

}	// namespace MCD

#endif	// __MCD_CORE_MATH_PLANE__
