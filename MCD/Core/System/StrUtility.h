#ifndef __SGE_CORE_SYSTEM_WSTR2STR__
#define __SGE_CORE_SYSTEM_WSTR2STR__

#include "../ShareLib.h"
#include "NonCopyable.h"
#include <string>

namespace SGE {

//! Convert narrow string into wide string using the current locale, return true on success.
SGE_CORE_API sal_checkreturn bool str2WStr(sal_in_z sal_notnull const char* narrowStr, std::wstring& wideStr);

//! Convert wide string into narrow string using the current locale, return true on success.
SGE_CORE_API sal_checkreturn bool wStr2Str(sal_in_z sal_notnull const wchar_t* wideStr, std::string& narrowStr);

SGE_CORE_API std::wstring str2WStr(const std::string& narrowStr);

SGE_CORE_API std::string wStr2Str(const std::wstring& wideStr);

//! Integer number to narrow string
SGE_CORE_API std::string int2Str(int number);

//! Integer number to wide string
SGE_CORE_API std::wstring int2WStr(int number);

//! Wide string to integer
SGE_CORE_API sal_checkreturn bool wStr2Int(sal_in_z sal_notnull const wchar_t* wideStr, int& number);

//! Wide string to double
SGE_CORE_API sal_checkreturn bool wStr2Double(sal_in_z sal_notnull const wchar_t* wideStr, double& number);

/*!	A simple name value pair parser.
	User supply a string that contains the name value pairs where name and value is
	linked up by the '=' character and each pair is separated by the ';' character.
	White space are skipped by the parser so user have to put quots (''' or '"') around
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
class SGE_CORE_API NvpParser : Noncopyable
{
public:
	NvpParser(sal_in_z sal_maybenull const wchar_t* str = nullptr);

	~NvpParser();

	void init(sal_in_z sal_notnull const wchar_t* str);

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

}	// namespace SGE

// Some wchar_t version of string functions are missing in cygwin
#ifdef SGE_CYGWIN

SGE_CORE_API wchar_t* wcsdup(const wchar_t* str);
SGE_CORE_API int wcscmp(const wchar_t* src, const wchar_t* dst);

#endif

#endif	// __SGE_CORE_SYSTEM_WSTR2STR__
