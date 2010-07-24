#include "Pch.h"
#include "../../../MCD/Core/System/StringHash.h"
#include "../../../MCD/Core/System/Utility.h"

using namespace MCD;

namespace {

struct TestData {
	size_t h;
	const char* v;
	size_t size;
};	// TestData

static TestData testData[] = {
	{ StringHash("", 0).hash, "", 0u },
	{ StringHash("Hello!").hash, "Hello!", 6u },
	{ StringHash("Ricky").hash, "Ricky", 5u },
	{ StringHash("M-Core").hash, "M-Core", 6u },
	{ StringHash("Simple is the best").hash, "Simple is the best", 18u  },
	{ StringHash("This is a longer string", 0).hash, "This is a longer string", 23u },
};

}	// namespace

TEST(FixStringTest)
{
	{	FixString tmp;
		CHECK(tmp.empty());
		CHECK_EQUAL(0u, tmp.size());
		CHECK_EQUAL('\0', tmp.c_str()[0]);
	}

	{	FixString tmp1, tmp2;
	}

	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		FixString s(testData[i].v);
		CHECK_EQUAL(0, strcmp(s.c_str(), testData[i].v));
		CHECK_EQUAL(testData[i].size, s.size());
	}

	FixString* fixStrings[MCD_COUNTOF(testData)] = { nullptr };
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		fixStrings[i] = new FixString(testData[i].v);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		delete fixStrings[i];

	// Null input string resulting an empty string
	CHECK_EQUAL(std::string(""), std::string(FixString((const char*)nullptr).c_str()));

	// Hash not found also result empty string
	CHECK_EQUAL(std::string(""), std::string(FixString(1234).c_str()));
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
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		const char* result = table.find(testData[i].h);
		CHECK(result && strcmp(result, testData[i].v) == 0);
	}

	// Remove the data
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		table.remove(testData[i].h);
		CHECK(!table.find(testData[i].h));
	}

	// Try different bucket size
	table.resizeBucket(2);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i)
		table.add(testData[i].h, testData[i].v);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		const char* result = table.find(testData[i].h);
		CHECK(result && strcmp(result, testData[i].v) == 0);
	}

	// Try an even larger bucket size
	table.resizeBucket(64);
	for(size_t i=0; i<MCD_COUNTOF(testData); ++i) {
		const char* result = table.find(testData[i].h);
		CHECK(result && strcmp(result, testData[i].v) == 0);
	}
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
