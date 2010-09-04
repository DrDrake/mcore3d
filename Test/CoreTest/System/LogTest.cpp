#include "Pch.h"
#include "../../../MCD/Core/System/Log.h"
#include <memory>   // For auto_ptr

using namespace MCD;

TEST(LogTest)
{
	{	// Just simple start and stop
		std::stringstream* s(new std::stringstream);
		Log::start(s);
		Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));
		Log::stop();
	}

	{	// Try to write a message
		std::stringstream* s(new std::stringstream);
		Log::start(s);

		const char msg[] = "Log testing";
		const char expected[] = "Info:  Log testing\n";
		Log::write(Log::Info, msg);
		CHECK_EQUAL(expected, s->str());

		Log::stop();
	}

	{	// Try to write formatted message
		std::stringstream* s(new std::stringstream);
		Log::start(s);

		const char expected[] = "Info:  Formatting 123, 456.789!!!\n";
		// Note that the format specification for "%s" and %S" are different
		// between unix and windows:
		// Linux: http://www.opengroup.org/onlinepubs/7990989799/xsh/fwprintf.html
		// Windows: http://msdn.microsoft.com/en-us/library/hf4y5e3w(VS.80).aspx
		Log::format(Log::Info, "Formatting %i, %3.3f%S", 123, 456.789, L"!!!");
		CHECK_EQUAL(expected, s->str());

		Log::stop();
	}
}
