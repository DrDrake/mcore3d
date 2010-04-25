#include "Pch.h"
#include "../../../MCD/Core/System/StringHash.h"
#include "../../../MCD/Core/System/Utility.h"

using namespace MCD;

namespace {

struct TestData {
	size_t h;
	const char* v;
};	// TestData

static TestData testData[] = {
	{ StringHash("Hello!").hash, "Hello!" },
	{ StringHash("Ricky").hash, "Ricky" },
	{ StringHash("M-Core").hash, "M-Core" },
	{ StringHash("Simple is the best").hash, "Simple is the best" },
	{ StringHash("This is a longer string", 0).hash, "This is a longer string" },
};

}	// namespace

TEST(FixStringTest)
{
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		FixString s(testData[i].v);
		CHECK_EQUAL(0, strcmp(s.c_str(), testData[i].v));
	}

	FixString* fixStrings[MCD_COUNTOF(testData)] = { nullptr };
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		fixStrings[i] = new FixString(testData[i].v);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		delete fixStrings[i];
}

TEST(StringHashSetTest)
{
	{	StringHashSet table;
	}

	{	StringHashSet table;
		table.resizeBucket(1);
		table.resizeBucket(0);
	}

	StringHashSet table;

	table.resizeBucket(1);

	// Adds some data
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		table.add(testData[i].h, testData[i].v);

	// Do nothing for adding the same string again
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		table.add(testData[i].h, testData[i].v);

	// Find the string by the hashed value
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		CHECK(strcmp(table.find(testData[i].h), testData[i].v) == 0);

	// Remove the data
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		table.remove(testData[i].h);
		CHECK(!table.find(testData[i].h));
	}

	// Try different bucket size
	table.resizeBucket(2);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		table.add(testData[i].h, testData[i].v);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		CHECK(strcmp(table.find(testData[i].h), testData[i].v) == 0);

	// Try an even larger bucket size
	table.resizeBucket(64);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		CHECK(strcmp(table.find(testData[i].h), testData[i].v) == 0);
}

TEST(Basic_StringHashTest)
{
	StringHash _1("1");
	StringHash _2("2");
	StringHash _2_("2");
	StringHash _21("21");
	StringHash _210("210");

	CHECK(_1 != _2);
	CHECK(_2 == _2_);
	CHECK(_2 != _21);

	const char* cstr = "210";
	const wchar_t* wstr = L"210";

	CHECK(_2 == StringHash(cstr, 1));
	CHECK(_21 == StringHash(cstr, 2));
	CHECK(_210 == StringHash(cstr, 3));

	CHECK(_2 == StringHash(wstr, 1));
	CHECK(_21 == StringHash(wstr, 2));
	CHECK(_210 == StringHash(wstr, 3));
}
