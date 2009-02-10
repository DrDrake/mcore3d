#pragma once

#include <string>

using namespace System;

namespace Binding
{
	public ref class Utility
	{
	public:
		static std::wstring toWString(String^ value);
	};
}
