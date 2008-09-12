#include "Pch.h"
#include "../../../MCD/Core/Math/Mat33.h"

using namespace MCD;

namespace
{
	const Mat33f ma(0, 1, 2, 3, 4, 5, 6, 7, 8);
	const Mat33f mb(8, 7, 6, 5, 4, 3, 2, 1, 0);
	const Mat33f mc(8);
	const Mat33f md(0, 2, 4, 6, 8, 10, 12, 14, 16);
}

TEST(Basic_Mat33Test)
{
	CHECK_EQUAL(ma.m01, ma[0][1]);
	CHECK_EQUAL(ma.data[1], ma[0][1]);

	CHECK_EQUAL(3u, ma.rows());
	CHECK_EQUAL(3u, ma.columns());

	CHECK(ma == ma);
	CHECK(ma != mb);
}

TEST(Copy_Mat33Test)
{
	Mat33f a(ma);
	CHECK(a == ma);

	Mat33f b(1);
	b = ma;
	CHECK(b == ma);
}

TEST(Equality_Mat33Test)
{
	CHECK(ma == ma);
	CHECK(!(ma != ma));
	CHECK(!(ma == 0));
	CHECK(!(0 == ma));
	CHECK(ma != 0);
	CHECK(0 != ma);
}

TEST(Indexer_Mat33Test)
{
	CHECK_EQUAL(0, ma[0][0]);
	CHECK_EQUAL(1, ma[0][1]);
	CHECK_EQUAL(2, ma[0][2]);
	CHECK_EQUAL(3, ma[1][0]);
	CHECK_EQUAL(4, ma[1][1]);
	CHECK_EQUAL(5, ma[1][2]);
	CHECK_EQUAL(6, ma[2][0]);
	CHECK_EQUAL(7, ma[2][1]);
	CHECK_EQUAL(8, ma[2][2]);
}

TEST(Negation_Mat33Test)
{
	CHECK(-ma == Mat33f(0, -1, -2, -3, -4, -5, -6, -7, -8));
}

TEST(Positive_Mat33Test)
{
	CHECK(+ma == ma);
}

TEST(Addition_Mat33Test)
{
	CHECK(ma + mb == mc);
	CHECK(mb + ma == mc);
	{	Mat33f t = ma;
		t += mb;
		CHECK(t == mc);
	}

	const Mat33f plus1(1, 2, 3, 4, 5, 6, 7, 8, 9);

	CHECK(ma + 1 == plus1);
	CHECK(1 + ma == plus1);
	{	Mat33f t = ma;
		t += 1;
		CHECK(t == plus1);
	}
}

TEST(Subtract_Mat33Test)
{
	CHECK(mc - mb == ma);
	CHECK(mc - ma == mb);
	{	Mat33f t = mc;
		t -= ma;
		CHECK(t == mb);
	}

	const Mat33f minus1(7, 6, 5, 4, 3, 2, 1, 0, -1);

	CHECK(mb - 1 == minus1);
	CHECK(1 - ma == Mat33f(1, 0, -1, -2, -3, -4, -5, -6, -7));
	{	Mat33f t = mb;
		t -= 1;
		CHECK(t == minus1);
	}

	CHECK(ma + ma - 2 * ma == 0);
}

TEST(Multiply_Mat33Test)
{
	CHECK(ma * 2 == md);
	CHECK(2 * ma == md);

	const Mat33f result(
		 9,  6,  3,
		54, 42, 30,
		99, 78, 57
	);

	CHECK(ma * mb == result);

	Mat33f t = ma;
	t *= 2;
	CHECK(t == md);

	t = ma;
	t *= mb;
	CHECK(t == result);
}

TEST(MulVector_Mat33Test)
{
	Vec3f u, v(1, 2, 3);
	u = ma * v;
	CHECK_CLOSE((ma * v).length(), u.length(), 1e-6);
	CHECK(u == Vec3f(8, 26, 44));
}

TEST(Division_Mat33Test)
{
	CHECK(md / 2 == ma);

	Mat33f t = md;
	t /= 2;
	CHECK(t == ma);
}

TEST(Rotate_Mat33Test)
{
	Mat33f m = Mat33f::rotate(Math<float>::cPiOver2(), 0, 0);
	Vec3f v = m * Vec3f::c001;

	CHECK(v.isNearEqual(-Vec3f::c010));
}
