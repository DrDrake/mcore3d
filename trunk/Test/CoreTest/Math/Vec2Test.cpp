#include "Pch.h"
#include "../../../MCD/Core/Math/Vec2.h"

using namespace MCD;

namespace {
	const Vec2f v10(1, 0);
	const Vec2f v01(0, 1);
	const Vec2f v11(1, 1);
	const Vec2f v12(1, 2);
}

TEST(Basic_Vec2Test)
{
	CHECK_EQUAL(1, v12.x);
	CHECK_EQUAL(2, v12.y);

	CHECK(v10 == v10);
	CHECK(v10 == Vec2f::c10);
	CHECK(v10 != v01);
}

TEST(Addition_Vec2Test)
{
	CHECK(v10 + v01 == v11);
	CHECK(v01 + v10 == v11);
	CHECK(v10 + v01 + v11 + v12 == Vec2f(3, 4));
	{	Vec2f a(v10);
		a += v01;
		CHECK(a == v11);
	}

	CHECK(v10 + 1 == Vec2f(2, 1));
	CHECK(1 + v10 == Vec2f(2, 1));
	{	Vec2f a(v10);
		a += 1;
		CHECK(a == Vec2f(2, 1));
	}
}

TEST(Subtract_Vec2Test)
{
	CHECK(v11 - v01 == v10);
	CHECK(v01 - v11 == -v10);
	{	Vec2f a(v11);
		a -= v01;
		CHECK(a == v10);
	}

	CHECK(v11 - 1 == Vec2f(0, 0));
	CHECK(1 - v11 == Vec2f(0, 0));
	{	Vec2f a(v11);
		a -= 1;
		CHECK(a == Vec2f(0, 0));
	}
}

TEST(Multiply_Vec2Test)
{
	CHECK(v11 * 2 == Vec2f(2, 2));
	CHECK(2 * v11 == Vec2f(2, 2));
	{	Vec2f a(v12);
		a *= 2;
		CHECK(a == Vec2f(2, 4));
	}
}

TEST(Division_Vec2Test)
{
	Vec2f r(0.5, 1);

	CHECK(v12 / 2 == r);
	CHECK(1.0 / (2.0 / v12) == r);

	Vec2f a(v12);
	a /= 2;
	CHECK(a == r);
}

TEST(Dot_Vec2Test)
{
	CHECK_EQUAL(2, v12 % v01);
	CHECK_EQUAL(2, v01 % v12);
	CHECK_EQUAL(5, v12 % v12);
	CHECK_EQUAL(4, (v01 + v01) % v12);

	CHECK_CLOSE(0, v01 % v10, 1e-6);
}

TEST(Distance_Vec2Test)
{
	CHECK_CLOSE(v10.distance(v01), v01.distance(v10), 1e-6);
}

TEST(Normalize_Vec2Test)
{
	{	Vec2f v = v12;
		CHECK_CLOSE(v12.length(), v.normalize(), 1e-6);	// normalize() will return the old length
		CHECK_CLOSE(1.0f, v.length(), 1e-6);
	}

	{	Vec2f v12Backup = v12;
		CHECK_CLOSE(1.0f, v12.normalizedCopy().length(), 1e-6);
		// The value of v12 should kept constant
		CHECK(v12 == v12Backup);
	}
}

TEST(Length_Vec2Test)
{
	CHECK_EQUAL(1, v10.length());
	CHECK(Mathf::isNearEqual(v12.length(), sqrt(5.0f)));

	CHECK_EQUAL(1, v10.squaredLength());
	CHECK_EQUAL(5, v12.squaredLength());
}
