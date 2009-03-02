#include "Pch.h"
#include "../../../MCD/Core/System/UserData.h"

using namespace MCD;

TEST(UserDataTest)
{
	UserData data;
	data.setPtr(new int(123));

	// Mis-matched type should return null
	CHECK(data.getPtr<float>() == nullptr);

	int* i = data.getPtr<int>();
	CHECK(i != nullptr);
	if(i)
		CHECK_EQUAL(123, *i);
}
