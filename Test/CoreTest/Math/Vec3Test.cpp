#include "Pch.h"
#include "../../../MCD/Core/Math/Vec3.h"

using namespace MCD;

namespace {
	const Vec3f v100(1, 0, 0);
	const Vec3f v010(0, 1, 0);
	const Vec3f v001(0, 0, 1);
	const Vec3f v110(1, 1, 0);
	const Vec3f v123(1, 2, 3);
}

TEST(Basic_Vec3Test)
{
	CHECK_EQUAL(1, v123.x);
	CHECK_EQUAL(2, v123.y);
	CHECK_EQUAL(3, v123.z);

	CHECK(v100 == v100);
	CHECK(v001 == Vec3f::c001);
	CHECK(v100 != v010);
}

TEST(Addition_Vec3Test)
{
	CHECK(v100 + v010 == v110);
	CHECK(v010 + v100 == v110);
	CHECK(v100 + v010 + v110 + v123 == Vec3f(3, 4, 3));
	{	Vec3f a(v100);
		a += v010;
		CHECK(a == v110);
	}

	CHECK(v100 + 1 == Vec3f(2, 1, 1));
	CHECK(1 + v100 == Vec3f(2, 1, 1));
	{	Vec3f a(v100);
		a += 1;
		CHECK(a == Vec3f(2, 1, 1));
	}
}

TEST(Subtract_Vec3Test)
{
	CHECK(v110 - v010 == v100);
	CHECK(v010 - v110 == -v100);
	{	Vec3f a(v110);
		a -= v010;
		CHECK(a == v100);
	}

	CHECK(v110 - 1 == Vec3f(0, 0, -1));
	CHECK(1 - v110 == Vec3f(0, 0, 1));
	{	Vec3f a(v110);
		a -= 1;
		CHECK(a == Vec3f(0, 0, -1));
	}
}

TEST(Multiply_Vec3Test)
{
	CHECK(v110 * 2 == Vec3f(2, 2, 0));
	CHECK(2 * v110 == Vec3f(2, 2, 0));
	{	Vec3f a(v123);
		a *= 2;
		CHECK(a == Vec3f(2, 4, 6));
	}
}

TEST(Division_Vec3Test)
{
	Vec3f r(0.5, 1, 1.5);

	CHECK(v123 / 2 == r);
	CHECK(1.0 / (2.0 / v123) == r);

	Vec3f a(v123);
	a /= 2;
	CHECK(a == r);
}

TEST(Dot_Vec3Test)
{
	CHECK(v123 % v010 == 2);
	CHECK(v010 % v123 == 2);
	CHECK(v123 % v123 == 14);
	CHECK((v010 + v010) % v123 == 4);

	CHECK_CLOSE(0, v010 % v100, 1e-6);

	CHECK_CLOSE(v123 % v123, v123.norm(), 1e-6);
}

TEST(Cross_Vec3Test)
{
	CHECK((v100 ^ v010) == Vec3f::c001);
	CHECK((v010 ^ v100) == -Vec3f::c001);
}

TEST(Distance_Vec3Test)
{
	CHECK_CLOSE(v100.distance(v010), v010.distance(v001), 1e-6);
	CHECK_CLOSE(v110.distance(v100), v110.distance(v010), 1e-6);
}

TEST(Normalize_Vec3Test)
{
	{	Vec3f v = v123;
		CHECK_CLOSE(v123.length(), v.normalize(), 1e-6);	// normalize() will return the old length
		CHECK_CLOSE(1.0f, v.length(), 1e-6);
	}

	CHECK_CLOSE(1.0f, v123.normalizedCopy().length(), 1e-6);
}

TEST(Length_Vec3Test)
{
	CHECK(v100.length() == 1);
	CHECK(Mathf::isNearEqual(v123.length(), sqrt(14.0f)));

	CHECK(v100.squaredLength() == 1);
	CHECK(v123.squaredLength() == 14);
}
