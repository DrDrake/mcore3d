#include "Pch.h"
#include "../../../MCD/Core/System/Utility.h"

using namespace MCD;

TEST(ThrowIfNull_UtilityTest)
{
	void* ptr = nullptr;
	CHECK_THROW(throwIfNull(ptr), std::runtime_error);
}

#include <list>
TEST(Foreach_UtilityTest)
{
	std::list<int> list;
	for(size_t i=0; i<10; ++i)
		list.push_back(i);

	{	// Value type as loop variable
		int counter = 0;
		MCD_FOREACH(int value, list) {
			CHECK_EQUAL(counter, value);
			++counter;
		}
	}

	{	// Reference type as loop variable
		const std::list<int>& constListRef = list;
		int counter = 0;

		MCD_FOREACH(const int& value, constListRef) {
			CHECK_EQUAL(counter, value);
			++counter;
		}

		CHECK_EQUAL(10, counter);
	}

	{	// Should be able to break though the loop
		int counter = 0;
		MCD_FOREACH(int value, list) {
			(void)value;
			++counter;

			if(counter > 3)
				break;

			// If the break command didn't work, the test fail
			if(counter > 3) {
				CHECK(false);
				return;
			}
		}
	}
}
