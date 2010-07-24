#include "stdafx.h"
#include "Utility.h"
#include "../../MCD/Core/System/StrUtility.h"

namespace Binding {

using namespace System;
using namespace System::Runtime::InteropServices;

std::string Utility::toUtf8String(String^ value)
{
	std::wstring wideStr = Utility::toWString(value);
	std::string ret;
	MCD_VERIFY(MCD::wStrToUtf8(wideStr, ret));
	return ret;
}

std::wstring Utility::toWString(String^ value)
{
	IntPtr chars = Marshal::StringToHGlobalUni(value);
	std::wstring converted = static_cast<const wchar_t*>(chars.ToPointer());
	Marshal::FreeHGlobal(chars);
	return converted;
}

String^ Utility::fromUtf8(const std::string& value)
{
	std::wstring wideStr;
	MCD_VERIFY(MCD::utf8ToWStr(value, wideStr));
	return gcnew String(wideStr.c_str());
}

}
