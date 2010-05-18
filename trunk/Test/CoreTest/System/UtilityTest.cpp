#include "Pch.h"
#include "../../../MCD/Core/System/ErrorCode.h"
#include "../../../MCD/Core/System/Exception.h"
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

TEST(ErrorCodeTest)
{
	int lastError = getLastError();
	setLastError(lastError);
	CHECK(true);
}

TEST(RuntimeErrorTest)
{
	{	RuntimeError error("Hello!");
		CHECK_EQUAL(std::string("Hello!"), std::string(error.what()));
		CHECK_EQUAL(std::wstring(L"Hello!"), std::wstring(error.wwhat()));
	}

	{	RuntimeError error(L"Hello!");
		CHECK_EQUAL(std::string("Hello!"), std::string(error.what()));
		CHECK_EQUAL(std::wstring(L"Hello!"), std::wstring(error.wwhat()));
	}
}

TEST(CountofTest)
{
	{	int a[10] = {};
		CHECK_EQUAL(10u, MCD_COUNTOF(a));
		(void)a;
	}

	{	// MCD_COUNTOF should be able to use as a compile-time constant
		int a[10] = {};
		int b[MCD_COUNTOF(a) * 2];
		CHECK_EQUAL(20u, MCD_COUNTOF(b));
		(void)a;
		(void)b;
	}

	{	// The following should generate warning or error
//		int* a = nullptr;
//		CHECK_EQUAL(123u, MCD_COUNTOF(a));
	}
}

TEST(SwapMemoryTest)
{
	float data1[] = { 1.23f, 4.56f };
	float data2[] = { 7.89f, 9.87f };

	swapMemory(data1, data2, sizeof(data1));

	CHECK_EQUAL(7.89f, data1[0]);
	CHECK_EQUAL(9.87f, data1[1]);
	CHECK_EQUAL(1.23f, data2[0]);
	CHECK_EQUAL(4.56f, data2[1]);
}
