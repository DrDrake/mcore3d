#include "Pch.h"
#include "../../../MCD/Core/Math/Tuple.h"

using namespace MCD;

struct Tuple3f : public MathTuple<float, 3, Tuple3f> {
	typedef MathTuple<float, 3, Tuple3f> super_type;
	Tuple3f() {}
	Tuple3f(param_type val) : super_type(val) {}
};

TEST(Tuple_TupleTest)
{
	Tuple3f v1;
	v1[0] = 1; v1[1] = 2; v1[2] = 3;
	Tuple3f v2;
	v2[0] = 3; v2[1] = 2; v2[2] = 1;
	CHECK(v1 != v2);

	{	Tuple3f v3(v1 + v2);
		Tuple3f v4 = 4.0f;
		CHECK(v3 == v4);

		v3 = v1;
		v3 += v2;
		CHECK(v3 == v4);
	}

	{	CHECK(v1 - v1 == 0.0f);

		Tuple3f v3(v1);
		v3 -= v1;
		CHECK(v3 == 0.0f);
	}

	{	Tuple3f v3(v1);
		v3 *= 1;
		CHECK(v3 == v1);
	}
}
