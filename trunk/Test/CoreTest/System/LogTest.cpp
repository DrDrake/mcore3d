#include "Pch.h"
#include "../../../MCD/Core/System/Log.h"

using namespace MCD;

TEST(LogTest)
{
	{	// Just simple start and stop
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());
		Log::stop();
		s.release();
	}

	{	// Try to write a message
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());

		const wchar_t msg[] = L"Log testing";
		const wchar_t expected[] = L"Info:  Log testing";
		Log::write(Log::Info, msg);
		CHECK_EQUAL(expected, s->str());

		Log::stop();
		s.release();
	}

	{	// Try to write formatted message
		std::auto_ptr<std::wstringstream> s(new std::wstringstream);
		Log::start(s.get());

		const wchar_t expected[] = L"Info:  Formatting 123, 456.789!!!";
		Log::format(Log::Info, L"Formatting %i, %3.3f%s", 123, 456.789, L"!!!");
		CHECK_EQUAL(expected, s->str());

		Log::stop();
		s.release();
	}
}
