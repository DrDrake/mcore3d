#include "Pch.h"
#include "../../../MCD/Core/System/SharedPtr.h"

using namespace MCD;

TEST(Basic_SharedPtrTest)
{
	typedef SharedPtr<int> Ptr;

	{	Ptr a(new int(123));
		CHECK_EQUAL(1u, a.referenceCount());

		CHECK_EQUAL(123, *a);
		// Destruction of a
	}

	{	// Test for equality
		Ptr a(new int(123));
		CHECK(a);
		CHECK(!a == false);

		Ptr b(a);
		CHECK(a == b);
		CHECK(a != b == false);

		Ptr c(new int(456));
		CHECK(a != c);
		CHECK(a == c == false);

		a = nullptr;
		CHECK(!a);
		CHECK(a == false);
	}

	{	// Swap
		Ptr a(new int(123));
		Ptr b(new int(456));

		swap(a, b);
		CHECK_EQUAL(456, *a);
		CHECK_EQUAL(123, *b);
	}
}
