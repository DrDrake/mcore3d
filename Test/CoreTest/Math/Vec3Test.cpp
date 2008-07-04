#include "Pch.h"
#include "../../../SGE/Core/Math/Vec3.h"

using namespace SGE;

namespace {
	const Vec3f v100(1, 0, 0);
	const Vec3f v010(0, 1, 0);
	const Vec3f v110(1, 1, 0);
	const Vec3f v123(1, 2, 3);
}

TEST(Addition_TVec3Test) {
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

TEST(Subtract_TVec3Test) {
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

TEST(Multiply_TVec3Test) {
	CHECK(v110 * 2 == Vec3f(2, 2, 0));
	CHECK(2 * v110 == Vec3f(2, 2, 0));
	{	Vec3f a(v123);
		a *= 2;
		CHECK(a == Vec3f(2, 4, 6));
	}
}

TEST(Division_TVec3Test) {
	Vec3f r(0.5, 1, 1.5);

	CHECK(v123 / 2 == r);
	CHECK(1.0 / (2.0 / v123) == r);

	Vec3f a(v123);
	a /= 2;
	CHECK(a == r);
}

TEST(Dot_TVec3Test) {
	CHECK(v123 % v010 == 2);
	CHECK(v010 % v123 == 2);
	CHECK(v123 % v123 == 14);
	CHECK((v010 + v010) % v123 == 4);
}

TEST(Cross_TVec3Test) {
	CHECK((v100 ^ v010) == Vec3f::c001);
	CHECK((v010 ^ v100) == -Vec3f::c001);
}

TEST(Length_TVec3Test) {
	CHECK(v100.length() == 1);
//	CHECK(IsNearEqual(TReal(v123.length()), Sqrt(14)));

	CHECK(v100.squaredLength() == 1);
	CHECK(v123.squaredLength() == 14);
}
