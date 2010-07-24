#include "Pch.h"
#include "../../../MCD/Core/System/PtrVector.h"

using namespace MCD;

namespace {

class Foo {
public:
	Foo(int id) : mId(id) {}
	int mId;
};	// Foo

typedef ptr_vector<Foo> Foos;

}	// namespace

TEST(PtrVectorTest)
{
	Foos foos;
	CHECK(foos.empty());
	CHECK_EQUAL(0u, foos.size());

	foos.push_back(new Foo(1));
	foos.push_back(new Foo(2));

	CHECK(!foos.empty());
	CHECK_EQUAL(2u, foos.size());
	CHECK_EQUAL(1, foos[0].mId);
	CHECK_EQUAL(2, foos[1].mId);
	CHECK_EQUAL(1, foos.front().mId);
	CHECK_EQUAL(2, foos.back().mId);

	// Re-inserting an existing elements with re_push_back
	foos.re_push_back(&foos[0]);
	CHECK_EQUAL(2, foos[0].mId);
	CHECK_EQUAL(1, foos[1].mId);

	foos.insert(foos.begin(), new Foo(3));
	CHECK_EQUAL(3, foos[0].mId);
	CHECK_EQUAL(2, foos[1].mId);
	CHECK_EQUAL(1, foos[2].mId);
	CHECK_EQUAL(3u, foos.size());

	foos.erase(foos.begin() + 1);
	CHECK_EQUAL(3, foos[0].mId);
	CHECK_EQUAL(1, foos[1].mId);
	CHECK_EQUAL(2u, foos.size());

	foos.pop_back();
	CHECK_EQUAL(3, foos[0].mId);
	CHECK_EQUAL(1u, foos.size());
}
