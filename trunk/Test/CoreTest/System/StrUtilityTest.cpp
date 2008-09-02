#include "Pch.h"
#include "../../../MCD/Core/System/StrUtility.h"

using namespace MCD;

TEST(WStr2StrTest)
{
	struct S {
		const char* narrowStr;
		const wchar_t* wideStr;
	};

	const S data[] = {
		{"",			L""},
#ifdef MCD_VC
		{"一二三 abc",	L"一二三 abc"},
		{"ABC",			L"ABC"},
		{"龍文達",		L"龍文達"},
#endif
	};

	::setlocale(LC_ALL, "");
//	printf("Using locale: %s\n", ::setlocale(LC_ALL, nullptr));

	{	// Narrow string to wide string
		std::wstring result;
		for(size_t i=0; i<sizeof(data)/sizeof(S); ++i) {
			CHECK(str2WStr(data[i].narrowStr, result));
			CHECK_EQUAL(data[i].wideStr, result);
			result = str2WStr(std::string(data[i].narrowStr));
			CHECK_EQUAL(data[i].wideStr, result);
		}
	}

	{	// Wide string to narrow string
		std::string result;
		for(size_t i=0; i<sizeof(data)/sizeof(S); ++i) {
			CHECK(wStr2Str(data[i].wideStr, result));
			CHECK_EQUAL(data[i].narrowStr, result);
			result = wStr2Str(std::wstring(data[i].wideStr));
			CHECK_EQUAL(data[i].narrowStr, result);
		}
	}
}

TEST(NumberToStrTest)
{
	CHECK_EQUAL("-123", int2Str(-123));
	CHECK_EQUAL(L"-123", int2WStr(-123));

	CHECK_EQUAL("0", int2Str(0));
	CHECK_EQUAL(L"0", int2WStr(0));

	CHECK_EQUAL("123", int2Str(123));
	CHECK_EQUAL(L"123", int2WStr(123));

	CHECK_EQUAL("1234567890", int2Str(1234567890));
	CHECK_EQUAL(L"1234567890", int2WStr(1234567890));
}

TEST(StrtoNumberTest)
{
	struct S {
		const wchar_t* wideStr;
		int number;
		bool success;
	};

	const S data[] = {
		{L"",			0,		false},
		{L"0",			0,		true},
		{L"123",		123,	true},
		{L"-123",		-123,	true},
		{L"ABC",		0,		false},	// Not a number
		{L"99999999999",0,		false},	// Overflow
	};

	for(size_t i=0; i<sizeof(data)/sizeof(S); ++i) {
		// Without default value
		int a;
		bool ok = wStr2Int(data[i].wideStr, a);
		CHECK(ok == data[i].success);
		if(ok)
			CHECK_EQUAL(data[i].number, a);

		// With default value
		a = wStr2IntWithDefault(data[i].wideStr, -123);
		CHECK_EQUAL(data[i].success ? data[i].number : -123, a);
	}
}

TEST(WStrCaseCmpTest)
{
	CHECK_EQUAL(0, wstrCaseCmp(L"abc", L"AbC"));
}

TEST(NvpParserTest)
{
	const wchar_t* str =
		L"x = '1';\n "
		L"y = 2; \t"
		L"z=1.23;"
		L"Empty='';"				// Representation of an empty string value
		L"ShortName =Ricky;"		// String without space ok
		L"FullName = 'Ricky Lung';"	// Space inside quot ok
		L"City = \"Hong Kong;\";"	// Another type of quot
		L"Text1 = \"'1', '2'\";"	// Quot inside string is ignored
		L"Text2 = '\"1\", \"2\"';";	// Quot inside string is ignored

	struct S {
		const wchar_t* name;
		const wchar_t* value;
	};

	const S data[] = {
		{ L"x",			L"1"},
		{ L"y",			L"2"},
		{ L"z",			L"1.23"},
		{ L"Empty",		L""},
		{ L"ShortName",	L"Ricky"},
		{ L"FullName",	L"Ricky Lung"},
		{ L"City",		L"Hong Kong;"},
		{ L"Text1",		L"'1', '2'"},
		{ L"Text2",		L"\"1\", \"2\""},
	};

	NvpParser parser(str);

	for(size_t i=0; i<sizeof(data)/sizeof(S); ++i) {
		const wchar_t* name = nullptr, *value = nullptr;
		CHECK(parser.next(name, value));

		if(!name || !value) {
			CHECK(false);
			continue;
		}
		
		CHECK_EQUAL(std::wstring(data[i].name), name);
		CHECK_EQUAL(std::wstring(data[i].value), value);
	}

	{	// No more pair to parse
		const wchar_t* name = nullptr, *value = nullptr;
		CHECK(!parser.next(name, value));
		CHECK(!name);
		CHECK(!value);
	}
}
