#include "Pch.h"
#include "Exception.h"
#include "StrUtility.h"

namespace MCD {

RuntimeError::RuntimeError(const char* msg)
	:
	std::runtime_error(""),
	mAMessage(msg)
{
}

RuntimeError::RuntimeError(const wchar_t* msg)
	:
	std::runtime_error(""),
	mWMessage(msg)
{
}

const char* RuntimeError::what() const throw()
{
	if(!mAMessage.empty())
		return mAMessage.c_str();

	if(wStrToStr(mWMessage.c_str(), mAMessage))
		return mAMessage.c_str();

	// Error message cannot be displayed
	return "";
}

const wchar_t* RuntimeError::wwhat() const throw()
{
	if(!mWMessage.empty())
		return mWMessage.c_str();

	if(strToWStr(mAMessage, mWMessage))
		return mWMessage.c_str();

	// Error message cannot be displayed
	return L"";
}

}	// namespace MCD
