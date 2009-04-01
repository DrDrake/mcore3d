#include "Pch.h"
#include "../../../MCD/Core/Math/Intersection.h"
#include "../../../MCD/Core/Math/Plane.h"
#include "../../../MCD/Core/Math/Ray.h"
#include <limits>

using namespace MCD;

TEST(RayPlane_IntersectionTest)
{
	{	Ray ray(Vec3f(0, 0, 0), Vec3f::c100);
		Plane plane(Vec3f::c100, 1);

		float distanceAlongRay;
		CHECK(Intersects(ray, plane, distanceAlongRay));
		CHECK_EQUAL(1, distanceAlongRay);
	}

	{	Ray ray(Vec3f(0, 0, 0), -Vec3f::c100);
		Plane plane(Vec3f::c100, 1);

		float distanceAlongRay;
		CHECK(Intersects(ray, plane, distanceAlongRay));
		CHECK_EQUAL(-1, distanceAlongRay);
	}

	{	Ray ray(Vec3f(0, 0, 0), Vec3f(1, 1, 0).normalizedCopy());
		Plane plane(Vec3f::c100, 1);

		float distanceAlongRay;
		CHECK(Intersects(ray, plane, distanceAlongRay));
		CHECK_EQUAL(Mathf::cSqrt2(), distanceAlongRay);
	}

	{	Ray ray(Vec3f(0, 0, 0), Vec3f(1, 1, 0).normalizedCopy());
		Plane plane(Vec3f::c100, -1);

		float distanceAlongRay;
		CHECK(Intersects(ray, plane, distanceAlongRay));
		CHECK_EQUAL(-Mathf::cSqrt2(), distanceAlongRay);
	}

	{	// Parallel, no intersection
		Ray ray(Vec3f(0, 0, 0), Vec3f::c100);
		Plane plane(Vec3f::c010, 1);

		float distanceAlongRay;
		CHECK(!Intersects(ray, plane, distanceAlongRay));
		CHECK_EQUAL(std::numeric_limits<float>::max(), distanceAlongRay);
	}
}
