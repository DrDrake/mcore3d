#include "Pch.h"
#include "Intersection.h"
#include "Plane.h"
#include "Ray.h"
#include <limits>

namespace MCD {

bool Intersects(const Ray& ray, const Plane& plane, float& distanceAlongRay)
{
	float denom = plane.normal.dot(ray.unitDirection);

	if(!Mathf::isNearZero(denom))
	{
		float nom = plane.normal.dot(ray.origin) + plane.constant;
		distanceAlongRay = nom / denom;
		return true;
	}

	// Parallel
	// Since negative max or positive max is so sensitive to precision, just
	// simply always return positive max will be ok
	distanceAlongRay = std::numeric_limits<float>::max();
	return false;
}

}	// namespace MCD
