#include "Pch.h"
#include "StrUtility.h"
#include "ErrorCode.h"
#include <map>
#include <sstream>
#include <stdlib.h>
#include <stdexcept>

#ifdef MCD_CYGWIN
#	include "PlatformInclude.h"
#endif

namespace MCD {

static const size_t cError = size_t(-1);

bool str2WStr(sal_in_z sal_notnull const char* narrowStr, std::wstring& wideStr)
{
#ifdef MCD_CYGWIN
	// Count also '\0'
	size_t count = ::MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, nullptr, 0);
	if(count <= 1)
		wideStr.clear();
	else {
		wideStr.resize(count-1);
		size_t converted = ::MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, (LPWSTR)&wideStr[0], count);
		if(converted == 0)
			return false;
		MCD_ASSERT(converted == count);
	}
#else
#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning (disable : 6309 6387)
#endif
	// Get the required character count of the destination string (\0 not included)
	size_t count = ::mbstowcs(nullptr, narrowStr, INT_MAX);
#ifdef MCD_VC
#	pragma warning(pop)
#endif

	// Check for error
	if(count == cError)
		return false;
	if(count == 0)
		wideStr.clear();
	else {
		wideStr.resize(count);
		// Warning: The underlying pointer of wideStr is being used
		size_t converted = ::mbstowcs(&(wideStr.at(0)), narrowStr, count+1);
		if(converted == 0 || converted == cError)
			return false;
		MCD_ASSERT(converted == count);
	}
#endif
	return true;
}

bool wStr2Str(sal_in_z sal_notnull const wchar_t* wideStr, std::string& narrowStr)
{
#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning (disable : 6309 6387)
#endif
	// Get the required character count of the destination string (\0 not included)
	size_t count = ::wcstombs(nullptr, wideStr, INT_MAX);
#ifdef MCD_VC
#	pragma warning(pop)
#endif

	// Check for error
	if(count == cError)
		return false;
	if(count == 0)
		narrowStr.clear();
	else {
		narrowStr.resize(count);
		// Warning: The underlying pointer of wideStr is being used
		size_t converted = ::wcstombs(&(narrowStr.at(0)), wideStr, count+1);
		if(converted == cError)
			return false;
		MCD_ASSERT(converted == count);
	}

	return true;
}

std::wstring str2WStr(const std::string& narrowStr)
{
	std::wstring wideStr;
	bool ok = str2WStr(narrowStr.c_str(), wideStr);
	if(!ok)
		throw std::runtime_error(
			MCD::getErrorMessage("Fail to convert narrow string to wide string: ", MCD::getLastError())
		);
	return wideStr;
}

std::string wStr2Str(const std::wstring& wideStr)
{
	std::string narrowStr;
	bool ok = wStr2Str(wideStr.c_str(), narrowStr);
	if(!ok)
		throw std::runtime_error(
			MCD::getErrorMessage("Fail to convert wide string to narrow string: ", MCD::getLastError())
		);
	return narrowStr;
}

std::string int2Str(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

std::wstring int2WStr(int number)
{
#ifdef MCD_CYGWIN
	std::stringstream ss;
	ss << number;
	return str2WStr(ss.str());
#else
	std::wstringstream ss;
	ss << number;
	return ss.str();
#endif
}

bool wStr2Int(const wchar_t* wideStr, int& number)
{
	// User sscanf or atoi didn't handle error very well
	// TODO: Use locale facet instead of stringstream
#ifdef MCD_CYGWIN
	std::stringstream ss(wStr2Str(wideStr));
#else
	std::wstringstream ss(wideStr);
#endif
	ss >> number;
	return !ss.fail();
}

int wStr2IntWithDefault(sal_in_z const wchar_t* wideStr, int defaultVal)
{
	if(!wideStr) return defaultVal;
	int ret;
	if(wStr2Int(wideStr, ret))
		return ret;
	return defaultVal;
}

int wstrCaseCmp(const wchar_t* string1, const wchar_t* string2)
{
#ifdef MCD_VC
	return ::_wcsicmp(string1, string2);
#elif defined(MCD_CYGWIN)
	wchar_t f, l;
	do {
		f = towlower(*string1);
		l = towlower(*string2);
		++string1;
		++string2;
	} while(f && (f == l));
	return (int)(f - l);
#else
	return ::wcscasecmp(string1, string2);
#endif
}

NvpParser::NvpParser(const wchar_t* str)
	: mStr(nullptr), mPos(nullptr)
{
	init(str);
}

void NvpParser::init(const wchar_t* str)
{
	::free(mStr);
	mStr = mPos = ::wcsdup(str);
}

NvpParser::~NvpParser()
{
	::free(mStr);
}

bool advancePos(wchar_t*& pos)
{
	if(*pos != L'\0') {
		++pos;
		return true;
	}
	return false;
}

static void skipSeps(wchar_t*& pos)
{
	static const wchar_t cSeps[] = L"; \t\n\r";
	static const size_t cCount = sizeof(cSeps)/sizeof(wchar_t) - 1;

	for(size_t i=cCount; i--;) {
		if(*pos != cSeps[i])
			continue;
		i = cCount;
		if(!advancePos(pos))
			return;
	}
}

void skipNonSeps(wchar_t*& pos)
{
	static const wchar_t cSeps[] = L"=; \t\n\r";
	static const size_t cCount = sizeof(cSeps)/sizeof(wchar_t) - 1;

	do {
		for(size_t i=cCount; i--;) {
			if(*pos == cSeps[i])
				return;
		}
	} while(advancePos(pos));
}

bool NvpParser::next(const wchar_t*& name, const wchar_t*& value)
{
	static const wchar_t cQuots[] = L"'\"";

	// Get the name
	skipSeps(mPos);
	wchar_t* name_ = mPos;
	skipNonSeps(mPos);

	if(*mPos != L'=' && advancePos(mPos)) {
		*(mPos-1) = L'\0';
		skipSeps(mPos);
	}

	// Should be '='
	if(*mPos != L'=')
		return false;
	*(mPos++) = L'\0';

	// Get the value
	skipSeps(mPos);
	// Get quoted string
	if(*mPos == cQuots[0] || *mPos == cQuots[1]) {
		wchar_t quot = *mPos;
		value = mPos + 1;
		while(advancePos(mPos) && *mPos != quot);
	} else {
		value = mPos;
		skipNonSeps(mPos);
	}

	if(advancePos(mPos))
		*(mPos-1) = L'\0';

	name = name_;
	return true;
}

}	// namespace MCD

#ifdef MCD_CYGWIN

wchar_t* wcsdup(const wchar_t* str)
{
	const wchar_t* p = str;
	while(*p != L'\0') { ++p; }
	size_t bytes = (p - str + 1) * sizeof(wchar_t);
	wchar_t* ret = reinterpret_cast<wchar_t*>(malloc(bytes));
	::memcpy(ret, str, bytes);
	return ret;
}

// Borrow from Visual Studio 2008 crt source code: wcscmp.c
int wcscmp(const wchar_t* src, const wchar_t* dst)
{
	int ret = 0 ;

	while(!(ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if(ret < 0)
		ret = -1;
	else if(ret > 0)
		ret = 1;

	return ret;
}

// TODO: Implementation
int vswprintf(wchar_t* dest, size_t count, const wchar_t* format, va_list argptr)
{
	memset(dest, 0, count * sizeof(wchar_t));
	return 0;
}

#endif	// #ifdef MCD_CYGWIN
