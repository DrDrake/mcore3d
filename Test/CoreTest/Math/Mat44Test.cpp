#include "Pch.h"
#include "../../../MCD/Core/Math/Mat44.h"

using namespace MCD;

namespace
{
	const Mat44f ma(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
	const Mat44f mb(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	const Mat44f mc(15);
	const Mat44f md(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
}

TEST(Basic_Mat44Test)
{
	CHECK_EQUAL(ma.m01, ma[0][1]);
	CHECK_EQUAL(ma.data[1], ma[0][1]);

	CHECK_EQUAL(4u, ma.rows());
	CHECK_EQUAL(4u, ma.columns());

	CHECK(ma == ma);
	CHECK(ma != mb);

	{	float buffer[4][4];
		ma.copyTo((float*)buffer);
		Mat44f m;
		m.copyFrom((float*)buffer);
		CHECK(m == ma);
	}
}

TEST(Copy_Mat44Test)
{
	Mat44f a(ma);
	CHECK(a == ma);

	Mat44f b(1);
	b = ma;
	CHECK(b == ma);
}

TEST(Equality_Mat44Test)
{
	CHECK(ma == ma);
	CHECK(!(ma != ma));
	CHECK(!(ma == 0));
	CHECK(!(0 == ma));
	CHECK(ma != 0);
	CHECK(0 != ma);
}

TEST(Indexer_Mat44Test)
{
	CHECK_EQUAL(0, ma[0][0]);
	CHECK_EQUAL(1, ma[0][1]);
	CHECK_EQUAL(2, ma[0][2]);
	CHECK_EQUAL(3, ma[0][3]);
	CHECK_EQUAL(4, ma[1][0]);
	CHECK_EQUAL(5, ma[1][1]);
	CHECK_EQUAL(6, ma[1][2]);
	CHECK_EQUAL(7, ma[1][3]);
	CHECK_EQUAL(8, ma[2][0]);
	CHECK_EQUAL(9, ma[2][1]);
	CHECK_EQUAL(10, ma[2][2]);
	CHECK_EQUAL(11, ma[2][3]);
	CHECK_EQUAL(12, ma[3][0]);
	CHECK_EQUAL(13, ma[3][1]);
	CHECK_EQUAL(14, ma[3][2]);
	CHECK_EQUAL(15, ma[3][3]);
}

TEST(Negation_Mat44Test)
{
	CHECK(-ma == Mat44f(0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15));
}

TEST(Positive_Mat44Test)
{
	CHECK(+ma == ma);
}

TEST(Addition_Mat44Test)
{
	CHECK(ma + mb == mc);
	CHECK(mb + ma == mc);
	{	Mat44f t = ma;
		t += mb;
		CHECK(t == mc);
	}

	const Mat44f plus1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

	CHECK(ma + 1 == plus1);
	CHECK(1 + ma == plus1);
	{	Mat44f t = ma;
		t += 1;
		CHECK(t == plus1);
	}
}

TEST(Subtract_Mat44Test)
{
	CHECK(mc - mb == ma);
	CHECK(mc - ma == mb);
	{	Mat44f t = mc;
		t -= ma;
		CHECK(t == mb);
	}

	const Mat44f minus1(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1);

	CHECK(mb - 1 == minus1);
	CHECK(1 - ma == Mat44f(1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14));
	{	Mat44f t = mb;
		t -= 1;
		CHECK(t == minus1);
	}

	CHECK(ma + ma - 2 * ma == 0);
}

TEST(Multiply_Mat44Test)
{
	CHECK(ma * 2 == md);
	CHECK(2 * ma == md);

	const Mat44f result(
		 34,  28,  22,  16,
		178, 156, 134, 112,
		322, 284, 246, 208,
		466, 412, 358, 304
	);

	CHECK(ma * mb == result);

	{	Mat44f m = ma;
		m = m * mb;
		CHECK(m == result);
	}

	Mat44f t = ma;
	t *= 2;
	CHECK(t == md);

	t = ma;
	t *= mb;
	CHECK(t == result);
}

TEST(MulVector_Mat44Test)
{
	Vec4f u, v(1, 2, 3, 4);
	u = ma * v;
	CHECK_CLOSE((ma * v).length(), u.length(), 1e-6);
	CHECK(u == Vec4f(20, 60, 100, 140));
}

TEST(Division_Mat44Test)
{
	CHECK(md / 2 == ma);

	Mat44f t = md;
	t /= 2;
	CHECK(t == ma);
}

TEST(Determinant_Mat44Test)
{
	Mat44f a(
		1, 2, 1, 1,
		2, 2, 2, 2,
		2, 1, 1, 2,
		2, 1, 0, 1);

	CHECK_EQUAL(-2, a.determinant());
	CHECK_EQUAL(-2, (a + 0).determinant());
}

TEST(Inverse_Mat44Test)
{
	Mat44f a(
		1, 2, 1, 1,
		2, 2, 2, 2,
		2, 1, 1, 2,
		2, 1, 0, 1);

	Mat44f b(
		-1,     1, -1,  1,
		 1, -0.5f,  0,  0,
		-1,  1.5f, -1,  0,
		 1, -1.5f,  2, -1);

	Mat44f ai;
	CHECK(a.inverse(ai));

	CHECK(ai == b);
	CHECK((a * 1).inverse() == ai);

	CHECK(a * ai == Mat44f::cIdentity);
	CHECK(ai * a == Mat44f::cIdentity);
}

TEST(Mat33_Mat44Test)
{
	Mat33f a(0, 1, 2, 3, 4, 5, 6, 7, 8);

	Mat44f b(a);
	CHECK(a == b.mat33());

	b = Mat44f::cIdentity;
	b.setMat33(a);
	CHECK(a == b.mat33());
}

TEST(Translation_Mat44Test)
{
	Mat44f m(ma);
	Mat33f m3 = m.mat33();
	CHECK(m.translation().isNearEqual(Vec3f(3, 7, 11)));

	m.translateBy(Vec3f(3, 2, 1));
	CHECK(m.translation().isNearEqual(Vec3f(6, 9, 12)));

	m.setTranslation(Vec3f(-1, -2, -3));
	CHECK(m.translation().isNearEqual(Vec3f(-1, -2, -3)));

	// The other parts of the matrix should not change.
	CHECK(m3.isNearEqual(m.mat33()));
}

TEST(Scale_Mat44Test)
{
	Mat44f m(Mat44f::cIdentity);

	CHECK(m.scale().isNearEqual(Vec3f(1)));

	m.scaleBy(Vec3f(2));
	CHECK(m.scale().isNearEqual(Vec3f(2)));
}

TEST(Rotation_Mat44Test)
{
	Mat44f m(Mat44f::cIdentity);

	// Make some noise to test the robustness of rotateBy()
	m.setScale(Vec3f(1, 2, 3));
	m.setTranslation(Vec3f(4, 5, 6));

	// Apply rotation that finally do nothing
	m.rotateBy(Vec3f::c001, Mathf::cPi());
	m.rotateBy(Vec3f::c001, Mathf::cPi());
	m.rotateBy(Vec3f::c001, Mathf::cPiOver2());
	m.rotateBy(Vec3f::c001, -Mathf::cPiOver2());

	m.setScale(Vec3f(1));
	m.setTranslation(Vec3f::cZero);

	CHECK(m.xBiasVector().isNearEqual(Mat44f::cIdentity.xBiasVector()));
	CHECK(m.yBiasVector().isNearEqual(Mat44f::cIdentity.yBiasVector()));
	CHECK(m.zBiasVector().isNearEqual(Mat44f::cIdentity.zBiasVector()));
}
