#ifndef __MCD_CORE_SYSTEM_wStrToStr__
#define __MCD_CORE_SYSTEM_wStrToStr__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include <string>

namespace MCD {

//! Convert narrow string into wide string using the current locale, return true on success.
MCD_CORE_API sal_checkreturn bool strToWStr(sal_in_z sal_in_ecount(maxCount) const char* narrowStr, size_t maxCount, std::wstring& wideStr);
MCD_CORE_API sal_checkreturn bool strToWStr(const std::string& narrowStr, std::wstring& wideStr);

//! Convert wide string into narrow string using the current locale, return true on success.
MCD_CORE_API sal_checkreturn bool wStrToStr(sal_in_z sal_in_ecount(maxCount) const wchar_t* wideStr, size_t maxCount, std::string& narrowStr);
MCD_CORE_API sal_checkreturn bool wStrToStr(const std::wstring& wideStr, std::string& narrowStr);

MCD_CORE_API std::wstring strToWStr(const std::string& narrowStr);

MCD_CORE_API std::string wStrToStr(const std::wstring& wideStr);

//! Convert utf-8 string into wide string, return true on success.
MCD_CORE_API sal_checkreturn bool utf8ToWStr(sal_in_z sal_in_ecount(maxCount) const char* utf8Str, size_t maxCount, std::wstring& wideStr);
MCD_CORE_API sal_checkreturn bool utf8ToWStr(const std::string& utf8Str, std::wstring& wideStr);

//! Convert wide string into utf-8 string, return true on success.
MCD_CORE_API sal_checkreturn bool wStrToUtf8(sal_in_z sal_in_ecount(maxCount) const wchar_t* wideStr, size_t maxCount, std::string& utf8Str);
MCD_CORE_API sal_checkreturn bool wStrToUtf8(const std::wstring& wideStr, std::string& utf8Str);

//! Number to string.
MCD_CORE_API std::string int2Str(int number);
MCD_CORE_API std::string float2Str(float number);
MCD_CORE_API std::string double2Str(double number);

//! Number array to string.
MCD_CORE_API std::string intArray2Str(sal_in sal_in_ecount(count) const int* numbers, size_t count);
MCD_CORE_API std::string floatArray2Str(sal_in sal_in_ecount(count) const float* numbers, size_t count);

//! String to number.
MCD_CORE_API sal_checkreturn bool str2Int(sal_in_z const char* str, int& number);
MCD_CORE_API sal_checkreturn bool str2Double(sal_in_z const char* str, double& number);

//! String to number with a supplied default value in case the parse failed.
MCD_CORE_API int str2IntWithDefault(sal_in_z const char* str, int defaultVal);
MCD_CORE_API int str2DoubleWithDefault(sal_in_z const char* str, double defaultVal);

/*!	String to array of number.
	\return Return null if parse failed. Using delete[] to cleanup the memory.
	\param size
		As input, it indicate the maximum number to parse, use 0 to indicate don't care;
		as an output it tells you the actual number of element parsed.
 */
MCD_CORE_API sal_maybenull int* strToIntArray(sal_in_z const char* str, sal_inout size_t& size);
MCD_CORE_API sal_maybenull float* strToFloatArray(sal_in_z const char* str, sal_inout size_t& size);

//! Compare two character strings, ignoring case.
MCD_CORE_API sal_checkreturn int strCaseCmp(sal_in_z const char* string1, sal_in_z const char* string2);

//! Compare two wide-character strings, ignoring case.
MCD_CORE_API sal_checkreturn int wstrCaseCmp(sal_in_z const wchar_t* string1, sal_in_z const wchar_t* string2);

/*!	A simple name value pair parser.
	User supply a string that contains the name value pairs where name and value is
	linked up by the '=' character and each pair is separated by the ';' character.
	White space are skipped by the parser so user have to put quotes (''' or '"') around
	the name or the value, if they are going to have white spaces inside.

	Example:
	\code
	const char_t* str = "x = 1; city = 'Hong Kong'";
	NvpParser parser(str);
	char_t* name, *value;
	parser.next(name, value);	// name = "x", value = "1"
	parser.next(name, value);	// name = "city", value = "Hong Kong"
	parser.next(name, value);	// return false
	\endcode
 */
class MCD_CORE_API NvpParser : Noncopyable
{
public:
	NvpParser(sal_in_z_opt const char* str = nullptr);

	~NvpParser();

	void init(sal_in_z_opt const char* str = nullptr);

	/*!	Get the next name/value pair.
		\param name Output parameter to get the name
		\param value Output parameter to get the corresponding value to \em name
		\return true on success, false if no more pair
	 */
	sal_checkreturn bool next(const char*& name, const char*& value);

protected:
	char* mStr;
	char* mPos;
};	// NvpParser

}	// namespace MCD

// Some wchar_t version of string functions are missing in cygwin
#ifdef MCD_CYGWIN

MCD_CORE_API wchar_t* wcsdup(const wchar_t* str);
MCD_CORE_API int wcscmp(const wchar_t* src, const wchar_t* dst);

#include <stdarg.h>
MCD_CORE_API int vswprintf(wchar_t* dest, size_t count, const wchar_t* format, va_list argptr);

#endif

#endif	// __MCD_CORE_SYSTEM_wStrToStr__
