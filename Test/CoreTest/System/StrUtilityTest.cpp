#include "Pch.h"
#include "../../../MCD/Core/System/StrUtility.h"
#include "../../../MCD/Core/System/Utility.h"
#include <string.h>	// For strlen

using namespace MCD;

TEST(wStrToStrTest)
{
	struct S {
		const char* narrowStr;
		const wchar_t* wideStr;
	};

	const S data[] = {
		{"",			L""},
		{"ABC",			L"ABC"},
	};

	::setlocale(LC_ALL, "");
//	printf("Using locale: %s\n", ::setlocale(LC_ALL, nullptr));

	{	// Narrow string to wide string
		std::wstring result;
		for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
			size_t len = strlen(data[i].narrowStr);
			CHECK(strToWStr(data[i].narrowStr, len, result));
			CHECK_EQUAL(data[i].wideStr, result);
			result = strToWStr(std::string(data[i].narrowStr));
			CHECK_EQUAL(data[i].wideStr, result);
		}
	}

	{	// Wide string to narrow string
		std::string result;
		for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
			CHECK(wStrToStr(data[i].wideStr, result));
			CHECK_EQUAL(data[i].narrowStr, result);
			result = wStrToStr(std::wstring(data[i].wideStr));
			CHECK_EQUAL(data[i].narrowStr, result);
		}
	}
}

TEST(Utf8Test)
{
	// My name Lung Man Tat in Chinese Traditional
	const byte_t cMyNameUtf8[] = { 0xE9, 0xBE, 0x8D, 0xE6, 0x96, 0x87, 0xE9, 0x81, 0x94, 0 };

	// TODO: Take care of endian issue
	const byte_t cMyNameUtf16[] = { 0x8D, 0x9F, 0x87, 0x65, 0x54, 0x90, 0, 0 };
	const int32_t cMyNameUcs4[] = { 40845, 25991, 36948, 0 };

	const wchar_t* p = sizeof(wchar_t) == 2 ? (const wchar_t*)cMyNameUtf16 : (const wchar_t*)cMyNameUcs4;
	std::string aExpected((const char*)cMyNameUtf8);
	std::wstring wExpected(p);

	std::string as;
	std::wstring ws;

	CHECK(utf8ToWStr(aExpected, ws));
	CHECK_EQUAL(wExpected, ws);

	CHECK(wStrToUtf8(wExpected, as));
	CHECK_EQUAL(aExpected, as);

	// Converting empty string
	CHECK(utf8ToWStr(std::string(), ws));
	CHECK(wStrToUtf8(std::wstring(), as));
}

TEST(NumberToStrTest)
{
	CHECK_EQUAL("-123", int2Str(-123));
	CHECK_EQUAL("0", int2Str(0));
	CHECK_EQUAL("123", int2Str(123));
	CHECK_EQUAL("1234567890", int2Str(1234567890));
}

TEST(StrtoNumberTest)
{
	struct S {
		const char* wideStr;
		int number;
		bool success;
	};

	const S data[] = {
		{"",			0,		false},
		{"0",			0,		true},
		{"123",			123,	true},
		{"-123",		-123,	true},
		{"ABC",			0,		false},	// Not a number
		{"99999999999",	0,		false},	// Overflow
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		// Without default value
		int a;
		bool ok = str2Int(data[i].wideStr, a);
		CHECK(ok == data[i].success);
		if(ok)
			CHECK_EQUAL(data[i].number, a);

		// With default value
		a = str2IntWithDefault(data[i].wideStr, -123);
		CHECK_EQUAL(data[i].success ? data[i].number : -123, a);
	}
}

TEST(StrtoFloatArrayTest)
{
	struct S {
		const char* wideStr;
		const float* values;
		size_t expectCount;
		size_t maxCount;
		bool success;
	};

	const float v1[] = {0};
	const float v2[] = {1, 2};
	const float v3[] = {1, 2, 3};

	const S data[] = {
		{"",		nullptr,	0u,	 0u, false},	// Exact count
		{"0",		v1,			1u,	 1u, true},		//
		{"1 2",		v2,			2u,	 2u, true},		//
		{"1 2 3",	v3,			3u,	 3u, true},		//
		{"",		nullptr,	0u,	10u, false},	// Bigger count
		{"0",		v1,			1u,	10u, true},		//
		{"1 2",		v2,			2u,	20u, true},		//
		{"",		nullptr,	0u,	 0u, false},	// Smaller count
		{"1 2",		v2,			1u,	 1u, true},		//
		{"1 2 3",	v3,			2u,	 2u, true},		//
		{"",		nullptr,	0u,	 0u, false},	// Don't care count
		{"abc",		nullptr,	0u,	 0u, false},	//
		{"0",		v1,			1u,	 0u, true},		//
		{"1 2",		v2,			2u,	 0u, true},		//
		{"1 2 3 a",	v3,			3u,	 0u, true},		//
	};

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		size_t size = data[i].maxCount;
		float* ret = strToFloatArray(data[i].wideStr, size);
		CHECK_EQUAL(data[i].success, ret != nullptr);
		CHECK_EQUAL(data[i].expectCount, size);

		if(ret) for(size_t j=0; j<data[i].expectCount; ++j)
			CHECK_EQUAL(data[i].values[j], ret[j]);

		delete[] ret;
	}
}

TEST(WStrCaseCmpTest)
{
	CHECK_EQUAL(0, wstrCaseCmp(L"abc", L"AbC"));
}

TEST(NvpParserTest)
{
	const char* str =
		"x = '1';\n "
		"y = 2; \t"
		"z=1.23;"
		"Empty='';"				// Representation of an empty string value
		"ShortName =Ricky;"		// String without space ok
		"FullName = 'Ricky Lung';"	// Space inside quot ok
		"City = \"Hong Kong;\";"	// Another type of quot
		"Text1 = \"'1', '2'\";"	// Quot inside string is ignored
		"Text2 = '\"1\", \"2\"';";	// Quot inside string is ignored

	struct S {
		const char* name;
		const char* value;
	};

	const S data[] = {
		{ "x",			"1"},
		{ "y",			"2"},
		{ "z",			"1.23"},
		{ "Empty",		""},
		{ "ShortName",	"Ricky"},
		{ "FullName",	"Ricky Lung"},
		{ "City",		"Hong Kong;"},
		{ "Text1",		"'1', '2'"},
		{ "Text2",		"\"1\", \"2\""},
	};

	NvpParser parser(str);

	for(size_t i=0; i<MCD_COUNTOF(data); ++i) {
		const char* name = nullptr, *value = nullptr;
		CHECK(parser.next(name, value));

		if(!name || !value) {
			CHECK(false);
			continue;
		}

		CHECK_EQUAL(std::string(data[i].name), name);
		CHECK_EQUAL(std::string(data[i].value), value);
	}

	{	// No more pair to parse
		const char* name = nullptr, *value = nullptr;
		CHECK(!parser.next(name, value));
		CHECK(!name);
		CHECK(!value);
	}
}
