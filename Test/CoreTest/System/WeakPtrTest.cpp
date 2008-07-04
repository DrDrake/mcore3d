#include "Pch.h"
#include "../../../MCD/Core/System/WeakPtr.h"

using namespace MCD;

class Base : public WeakPtrTarget {};

class Derived : public Base {};

typedef WeakPtr<Base> BasePointer;

TEST(WeakPtrTest)
{
	// Plain usage
	Base b;
	BasePointer wpB(&b);
	CHECK_EQUAL(wpB.get(), &b);

	{	// Default constructor
		BasePointer wpB1;
		CHECK(wpB1.get() == nullptr);

		// Plain assignment
		Base b1;
		wpB1 = &b1;
		CHECK_EQUAL(wpB1.get(), &b1);
		wpB = wpB1;
		CHECK_EQUAL(wpB.get(), &b1);

		// Assignment to self
		wpB = wpB;
		CHECK_EQUAL(wpB.get(), wpB1.get());
	}

	{	// Assignment operator with casting
		Derived d;
		wpB = &d;
		CHECK_EQUAL(wpB.get(), &d);

		// Copy constructor with casting
		BasePointer wpBD(&d);
		CHECK_EQUAL(wpBD.get(), &d);

		// Destruction of target
	}
	CHECK(wpB.get() == nullptr);

	{	Derived d1;
		BasePointer temp1(&d1);

		// Swap valid/invalid pointers
		swap(wpB, temp1);
		CHECK_EQUAL(wpB.get(), (Base*)(&d1));
		CHECK(temp1.get() == nullptr);

		BasePointer temp2;
		temp1 = &d1;

		{	Derived d2;
			temp2 = &d2;

			// Swap valid/valid pointers
			temp1.swap(temp2);

			CHECK_EQUAL(temp1.get(), (Base*)(&d2));
			CHECK_EQUAL(temp2.get(), (Base*)(&d1));
		}

		CHECK(temp1.get() == nullptr);
		CHECK(temp2.get() != nullptr);
	}
}
