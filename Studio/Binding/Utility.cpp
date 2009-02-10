#include "stdafx.h"
#include "Utility.h"

namespace Binding {

using namespace System;
using namespace System::Runtime::InteropServices;

std::wstring Utility::toWString(String^ value)
{
	IntPtr chars = Marshal::StringToHGlobalUni(value);
	std::wstring converted = static_cast<const wchar_t*>(chars.ToPointer());
	Marshal::FreeHGlobal(chars);
	return converted;
}

}
