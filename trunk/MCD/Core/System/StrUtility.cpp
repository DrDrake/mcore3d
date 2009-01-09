#include "Pch.h"
#include "StrUtility.h"
#include "ErrorCode.h"
#include <map>
#include <sstream>
#include <stdlib.h>
#include <stdexcept>
#include <vector>

#ifdef MCD_CYGWIN
#	include "PlatformInclude.h"
#endif

namespace MCD {

static const size_t cError = size_t(-1);

bool strToWStr(sal_in_z sal_notnull const char* narrowStr, size_t maxCount, std::wstring& wideStr)
{
#ifdef MCD_CYGWIN
	// Count also '\0'
	size_t count = ::MultiByteToWideChar(CP_ACP, 0, narrowStr, maxCount, nullptr, 0);
	if(count <= 1)
		wideStr.clear();
	else {
		wideStr.resize(count-1);
		if(wideStr.size() != count-1)
			return false;
		size_t converted = ::MultiByteToWideChar(CP_ACP, 0, narrowStr, maxCount, (LPWSTR)&wideStr[0], count);
		if(converted == 0)
			return false;
		MCD_ASSERT(converted == count);
	}
#else
	// Get the required character count of the destination string (\0 not included)
	size_t count = ::mbstowcs(nullptr, narrowStr, maxCount);

	// Check for error
	if(count == cError)
		return false;
	if(count == 0)
		wideStr.clear();
	else {
		wideStr.resize(count);
		if(wideStr.size() != count)
			return false;

		size_t converted = ::mbstowcs(&(wideStr.at(0)), narrowStr, count+1);
		if(converted == 0 || converted == cError)
			return false;
		MCD_ASSERT(converted == count);
	}
#endif
	return true;
}

bool strToWStr(const std::string& narrowStr, std::wstring& wideStr)
{
	return strToWStr(narrowStr.c_str(), narrowStr.size(), wideStr);
}

bool wStrToStr(sal_in_z sal_notnull const wchar_t* wideStr, size_t maxCount, std::string& narrowStr)
{
	// Get the required character count of the destination string (\0 not included)
	size_t count = ::wcstombs(nullptr, wideStr, maxCount);

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

bool wStrToStr(const std::wstring& wideStr, std::string& narrowStr)
{
	return wStrToStr(wideStr.c_str(), wideStr.size(), narrowStr);
}

std::wstring strToWStr(const std::string& narrowStr)
{
	std::wstring wideStr;
	bool ok = strToWStr(narrowStr, wideStr);
	if(!ok)
		throw std::runtime_error(
			MCD::getErrorMessage("Fail to convert narrow string to wide string: ", MCD::getLastError())
		);
	return wideStr;
}

std::string wStrToStr(const std::wstring& wideStr)
{
	std::string narrowStr;
	bool ok = wStrToStr(wideStr, narrowStr);
	if(!ok)
		throw std::runtime_error(
			MCD::getErrorMessage("Fail to convert wide string to narrow string: ", MCD::getLastError())
		);
	return narrowStr;
}

// Reference: http://en.wikipedia.org/wiki/Utf8
static const byte_t cUtf8Limits[] = {
	0xC0,	// Start of a 2-byte sequence
	0xE0,	// Start of a 3-byte sequence
	0xF0,	// Start of a 4-byte sequence
	0xF8,	// Start of a 5-byte sequence
	0xFC,	// Start of a 6-byte sequence
	0xFE	// Invalid: not defined by original UTF-8 specification
};

/*!	Usually it is a 2 steps process to convert the string, invoke utf8ToUtf16() with
	dest equals to null so that it gives you destLen (not including null terminator),
	then allocate the destination with that amount of memory and call utf8ToUtf16() once
	again to perform the actual conversion. You can skip the first call if you sure
	the destination buffer is large enough to store the data.

	\ref Modify from 7zip LZMA sdk
 */
static bool utf8ToUtf16(wchar_t* dest, size_t& destLen, const char* src, size_t maxSrcLen)
{
	size_t destPos = 0, srcPos = 0;

	while(true)
	{
		byte_t c;	// Note that byte_t should be unsigned
		size_t numAdds;

		if(srcPos == maxSrcLen || src[srcPos] == '\0') {
			if(dest && destLen != destPos) {
				MCD_ASSERT(false && "The provided destLen should equals to what we calculated here");
				return false;
			}

			destLen = destPos;
			return true;
		}

		c = src[srcPos++];

		if(c < 0x80) {	// 0-127, US-ASCII (single byte)
			if(dest)
				dest[destPos] = (wchar_t)c;
			++destPos;
			continue;
		}

		if(c < 0xC0)	// The first octet for each code point should within 0-191
			break;

		for(numAdds = 1; numAdds < 5; ++numAdds)
			if(c < cUtf8Limits[numAdds])
				break;
		uint32_t value = c - cUtf8Limits[numAdds - 1];

		do {
			byte_t c2;
			if(srcPos == maxSrcLen || src[srcPos] == '\0')
				break;
			c2 = src[srcPos++];
			if(c2 < 0x80 || c2 >= 0xC0)
				break;
			value <<= 6;
			value |= (c2 - 0x80);
		} while(--numAdds != 0);

		if(value < 0x10000) {
			if(dest)
				dest[destPos] = (wchar_t)value;
			++destPos;
		}
		else {
			value -= 0x10000;
			if(value >= 0x100000)
				break;
			if(dest) {
				dest[destPos + 0] = (wchar_t)(0xD800 + (value >> 10));
				dest[destPos + 1] = (wchar_t)(0xDC00 + (value & 0x3FF));
			}
			destPos += 2;
		}
	}

	destLen = destPos;
	return false;
}

bool utf8ToWStr(const char* utf8Str, size_t maxCount, std::wstring& wideStr)
{
	size_t destLen = 0;

	// Get the length of the wide string
	if(!utf8ToUtf16(nullptr, destLen, utf8Str, maxCount))
		return false;

	wideStr.resize(destLen);
	if(wideStr.size() != destLen)
		return false;

	return utf8ToUtf16(const_cast<wchar_t*>(wideStr.c_str()), destLen, utf8Str, maxCount);
}

bool utf8ToWStr(const std::string& utf8Str, std::wstring& wideStr)
{
	return utf8ToWStr(utf8Str.c_str(), utf8Str.size(), wideStr);
}

//! See the documentation for utf8ToUtf16()
static bool utf16ToUtf8(char* dest, size_t& destLen, const wchar_t* src, size_t maxSrcLen)
{
	size_t destPos = 0, srcPos = 0;

	while(true)
	{
		uint32_t value;
		size_t numAdds;

		if(srcPos == maxSrcLen || src[srcPos] == L'\0') {
			if(dest && destLen != destPos) {
				MCD_ASSERT(false && "The provided destLen should equals to what we calculated here");
				return false;
			}
			destLen = destPos;
			return true;
		}

		value = src[srcPos++];

		if(value < 0x80) {	// 0-127, US-ASCII (single byte)
			if(dest)
				dest[destPos] = char(value);
			++destPos;
			continue;
		}

		if(value >= 0xD800 && value < 0xE000) {
			if(value >= 0xDC00 || srcPos == maxSrcLen)
				break;
			uint32_t c2 = src[srcPos++];
			if(c2 < 0xDC00 || c2 >= 0xE000)
				break;
			value = ((value - 0xD800) << 10) | (c2 - 0xDC00);
		}

		for(numAdds = 1; numAdds < 5; ++numAdds)
			if(value < (uint32_t(1) << (numAdds * 5 + 6)))
				break;

		if(dest)
			dest[destPos] = char(cUtf8Limits[numAdds - 1] + (value >> (6 * numAdds)));
		++destPos;

		do {
			--numAdds;
			if(dest)
				dest[destPos] = char(0x80 + ((value >> (6 * numAdds)) & 0x3F));
			++destPos;
		} while(numAdds != 0);
	}

	destLen = destPos;
	return false;
}

bool wStrToUtf8(const wchar_t* wideStr, size_t maxCount, std::string& utf8Str)
{
	size_t destLen = 0;

	// Get the length of the utf-8 string
	if(!utf16ToUtf8(nullptr, destLen, wideStr, maxCount))
		return false;

	utf8Str.resize(destLen);
	if(utf8Str.size() != destLen)
		return false;

	return utf16ToUtf8(const_cast<char*>(utf8Str.c_str()), destLen, wideStr, maxCount);
}

bool wStrToUtf8(const std::wstring& wideStr, std::string& utf8Str)
{
	return wStrToUtf8(wideStr.c_str(), wideStr.size(), utf8Str);
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
	return strToWStr(ss.str());
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
	std::stringstream ss(wStrToStr(wideStr));
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

bool wStr2Double(const wchar_t* wideStr, double& number)
{
	// User sscanf or atoi didn't handle error very well
	// TODO: Use locale facet instead of stringstream
#ifdef MCD_CYGWIN
	std::stringstream ss(wStrToStr(wideStr));
#else
	std::wstringstream ss(wideStr);
#endif
	ss >> number;
	return !ss.fail();
}

float* wStr2Float(const wchar_t* wideStr, size_t& size)
{
	const size_t maxSize = size;
	size = 0;
	float* ret = nullptr;
	std::vector<float> buffer;

	std::wstringstream ss(wideStr);

	for(size=0; maxSize == 0 || size < maxSize; ++size) {
		float number;
		ss >> number;
		if(ss.fail())
			break;
		buffer.push_back(number);
	}

	MCD_ASSERT(size == buffer.size());

	if(buffer.empty())
		return ret;

	ret = new float[buffer.size()];
	memcpy(ret, &buffer[0], sizeof(float) * buffer.size());
	return ret;
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
