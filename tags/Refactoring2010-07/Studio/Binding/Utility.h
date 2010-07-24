#pragma once

#include <string>

using namespace System;

namespace Binding
{
	public ref class Utility
	{
	public:
		static std::string toUtf8String(String^ value);
		static std::wstring toWString(String^ value);
		static String^ fromUtf8(const std::string& value);
	};
}
