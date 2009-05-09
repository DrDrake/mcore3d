#include "Pch.h"
#include "../../../MCD/Core/System/Log.h"
#include <memory>   // For auto_ptr

using namespace MCD;

TEST(LogTest)
{
	{	// Just simple start and stop
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());
		Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));
		Log::stop();
		s.release();
	}

	{	// Try to write a message
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());

		const wchar_t msg[] = L"Log testing";
		const wchar_t expected[] = L"Info:  Log testing\n";
		Log::write(Log::Info, msg);
		CHECK_EQUAL(expected, s->str());

		Log::stop();
		s.release();
	}

	{	// Try to write formatted message
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());

		const wchar_t expected[] = L"Info:  Formatting 123, 456.789!!!\n";
		// Note that the format specification for "%s" and %S" are different
		// between unix and windows:
		// Linux: http://www.opengroup.org/onlinepubs/7990989799/xsh/fwprintf.html
		// Windows: http://msdn.microsoft.com/en-us/library/hf4y5e3w(VS.80).aspx
#ifdef MCD_WIN32
		Log::format(Log::Info, L"Formatting %i, %3.3f%s", 123, 456.789, L"!!!");
#else
		Log::format(Log::Info, L"Formatting %i, %3.3f%S", 123, 456.789, L"!!!");
#endif
		CHECK_EQUAL(expected, s->str());

		Log::stop();
		s.release();
	}
}
