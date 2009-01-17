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

//! Integer number to narrow string.
MCD_CORE_API std::string int2Str(int number);

//! Integer number to wide string.
MCD_CORE_API std::wstring int2WStr(int number);

//! Wide string to integer.
MCD_CORE_API sal_checkreturn bool wStr2Int(sal_in_z const wchar_t* wideStr, int& number);

//! Wide string to integer with a supplied default value in case the parse failed.
MCD_CORE_API int wStr2IntWithDefault(sal_in_z const wchar_t* wideStr, int defaultVal);

//! Wide string to double.
MCD_CORE_API sal_checkreturn bool wStr2Double(sal_in_z const wchar_t* wideStr, double& number);

/*!	Wide string to array of integer.
	\return Return null if parse failed. Using delete[] to cleanup the memory.
	\param size
		As input, it indicate the maximum number to parse, use 0 to indicate don't care;
		as an output it tells you the actual number of element parsed.
 */
MCD_CORE_API int* wStrToIntArray(sal_in_z const wchar_t* wideStr, sal_inout size_t& size);

/*!	Wide string to array of float.
	\return Return null if parse failed. Using delete[] to cleanup the memory.
	\param size
		As input, it indicate the maximum number to parse, use 0 to indicate don't care;
		as an output it tells you the actual number of element parsed.
 */
MCD_CORE_API float* wStrToFloatArray(sal_in_z const wchar_t* wideStr, sal_inout size_t& size);

//! Compare two wide-character strings, ignoring case.
MCD_CORE_API sal_checkreturn int wstrCaseCmp(sal_in_z const wchar_t* string1, sal_in_z const wchar_t* string2);

/*!	A simple name value pair parser.
	User supply a string that contains the name value pairs where name and value is
	linked up by the '=' character and each pair is separated by the ';' character.
	White space are skipped by the parser so user have to put quotes (''' or '"') around
	the name or the value, if they are going to have white spaces inside.

	Example:
	\code
	const wchar_t* str = L"x = 1; city = 'Hong Kong'";
	NvpParser parser(str);
	wchar_t* name, *value;
	parser.next(name, value);	// name = "x", value = "1"
	parser.next(name, value);	// name = "city", value = "Hong Kong"
	parser.next(name, value);	// return false
	\endcode
 */
class MCD_CORE_API NvpParser : Noncopyable
{
public:
	NvpParser(sal_in_z_opt const wchar_t* str = nullptr);

	~NvpParser();

	void init(sal_in_z_opt const wchar_t* str = nullptr);

	/*!	Get the next name/value pair.
		\param name Output parameter to get the name
		\param value Output parameter to get the corresponding value to \em name
		\return true on success, false if no more pair
	 */
	sal_checkreturn bool next(const wchar_t*& name, const wchar_t*& value);

protected:
	wchar_t* mStr;
	wchar_t* mPos;
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
