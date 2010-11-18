#include "Pch.h"
#include "../../../MCD/Core/Math/Quaternion.h"

using namespace MCD;

TEST(Basic_QuaternionTest)
{
	{	Quaternionf q(1, 2, 3, 4);
		CHECK_EQUAL(1, q.x);
		CHECK_EQUAL(2, q.y);
		CHECK_EQUAL(3, q.z);
		CHECK_EQUAL(4, q.w);
	}

	{	Quaternionf q(Vec3f(1, 2, 3), 4);
		CHECK_EQUAL(1, q.x);
		CHECK_EQUAL(2, q.y);
		CHECK_EQUAL(3, q.z);
		CHECK_EQUAL(4, q.w);

		CHECK(q.getVec3() == Vec3f(1, 2, 3));

		q.setVec3(Vec3f(3, 2, 1));
		CHECK(q.getVec3() == Vec3f(3, 2, 1));
	}

	{	Quaternionf q(Vec4f(1, 2, 3, 4));
		CHECK_EQUAL(1, q.x);
		CHECK_EQUAL(2, q.y);
		CHECK_EQUAL(3, q.z);
		CHECK_EQUAL(4, q.w);
	}

	{	// Conjugate
		Quaternionf q(1, 2, 3, 4);
		q = q.conjugate();
		CHECK(q == Quaternionf(-1, -2, -3, 4));
	}

	{	// Inverse
		Quaternionf q(1, 2, 3, 4);
		CHECK(q.isNearEqual(q.inverse().inverse()));

		q /= q.length();
		CHECK(q.isNearEqual(q.inverseUnit().inverseUnit()));
	}

	{	// Creating and applying diff
		Quaternionf master, target;
		master.fromAxisAngle(Vec3f::c010, 0.3f * Mathf::cPi());
		target.fromAxisAngle(Vec3f(1,1,0), 0.2f * Mathf::cPi());

		const Quaternionf diff = master.inverse() * target;

		Quaternionf q = master * diff;
		CHECK(q.isNearEqual(target));
	}

	{	// Inverse with zero length
		Quaternionf q(0, 0, 0, 0);
		CHECK(q.isNearEqual(q.inverse().inverse()));
	}

	{	// The following 2 representation are consider the same
		Quaternionf q1( 1,  1,  1, -1);
		Quaternionf q2(-1, -1, -1, 1);
		CHECK(q1.isNearEqual(q2));
	}
}

TEST(AxisAngle_QuaternionTest)
{
	Quaternionf q;
	// Rotate around y-axis 45 degree anti-clockwise
	q.fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());
	// A pure rotational quaternion has unit length
	CHECK_CLOSE(1, q.length(), 1e-6);

	{	Vec3f v = Vec3f::c001;
		q.transform(v);
		CHECK(v.isNearEqual(Vec3f::c100));
	}

	{	Vec3f v;
		float angle;
		q.toAxisAngle(v, angle);
		CHECK(v.isNearEqual(Vec3f::c010));
		CHECK_CLOSE(Mathf::cPiOver2(), angle, 1e-6);
	}

	{	// Test for zero angle
		Quaternionf q2;
		q2.fromAxisAngle(Vec3f::c010, 0);
		Vec3f v;
		float angle;
		q2.toAxisAngle(v, angle);

		// Now v can be any unit vector
		CHECK_CLOSE(1, v.length(), 1e-6);
		CHECK_EQUAL(0, angle);
	}
}

TEST(Matrix_QuaternionTest)
{
	Quaternionf q;
	Mat33f m1;
	// Rotate around y-axis 90 degree anti-clockwise
	q.fromAxisAngle(Vec3f::c010, Mathf::cPiOver2());
	q.toMatrix(m1);

	Quaternionf q2;
	q2.fromMatrix(m1);
	CHECK(q.isNearEqual(q2));

	Mat33f m2 = Mat33f::makeXYZRotation(0, Mathf::cPiOver2(), 0);
	CHECK(m1.isNearEqual(m2));
}

TEST(Transform_QuaternionTest)
{
	const Vec3f v(10, 5, 1);

	for(int x = -5; x <= 5; ++x) for (int y = -5; y <= 5; ++y) for (int z = -5; z <= 5; ++z) {
		if(x == 0 && y == 0 && z == 0)
			continue;

		Vec3f qv = Vec3f(float(x), float(y), float(z));
		qv.normalize();//.normalize();

		for(int angle = -10; angle <= 10; ++angle) {
			Quaternionf q;
			q.fromAxisAngle(qv, angle * Mathf::cPi() * 2 / 10);
			Vec3f a = v;
			q.transform(a);
			Vec3f b = a;
			q.conjugate().transform(b);
			CHECK(v.isNearEqual(b, 1e-5f));
		}
	}
}
