#include "Pch.h"
#include "Exception.h"
#include "StrUtility.h"

namespace SGE {

RuntimeError::RuntimeError(const wchar_t* msg)
	:
	std::runtime_error(""),
	mMessage(msg)
{
}

const char* RuntimeError::what() const throw()
{
	if(mMessage.empty())
		return std::runtime_error::what();

	std::string narrowStr;

	if(wStr2Str(mMessage.c_str(), narrowStr))
		return narrowStr.c_str();

	// Error message cannot be displayed
	return "";
}

const wchar_t* RuntimeError::wwhat() const throw()
{
	if(!mMessage.empty())
		return mMessage.c_str();

	std::wstring wideStr;

	if(str2WStr(std::runtime_error::what(), wideStr))
		return wideStr.c_str();

	// Error message cannot be displayed
	return L"";
}

}	// namespace SGE
