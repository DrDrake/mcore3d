#include "Pch.h"
#include "Timer.h"
#include "PlatformInclude.h"
#include "StaticAssert.h"
#include <limits.h>	// For ULLONG_MAX
#include <math.h>	// For floof

namespace SGE {

uint64_t getTimeSinceMachineStartup()
{
	uint64_t ret;

#ifdef _WIN32
	::QueryPerformanceCounter((LARGE_INTEGER*)(&ret));
#else
	timeval tv;
	::gettimeofday(&tv, nullptr);
	ret = reinterpret_cast<const uint64_t&>(tv);
#endif

	return ret;
}

#ifdef _WIN32

uint64_t GetQueryPerformanceFrequency()
{
	LARGE_INTEGER ret;
	BOOL ok = ::QueryPerformanceFrequency(&ret);
	(void)ok;
	SGE_ASSERT(ok && "QueryPerformanceFrequency failed");
	SGE_STATIC_ASSERT(sizeof(uint64_t) == sizeof(ret.QuadPart));
	return uint64_t(ret.QuadPart);
}

static uint64_t cTicksPerSecond = GetQueryPerformanceFrequency();
static double cInvTicksPerSecond = 1.0 / GetQueryPerformanceFrequency();

void TimeInterval::set(double sec) {
	mTicks = uint64_t(sec * cTicksPerSecond);
}

double TimeInterval::asSecond() const {
	return mTicks * cInvTicksPerSecond;
}

#else

void TimeInterval::set(double sec) {
	timeval& tv = reinterpret_cast<timeval&>(mTicks);
	tv.tv_sec = uint32_t(::floorf(sec));
	tv.tv_usec = uint32_t((sec - tv.tv_sec) * 1e6);
}

double TimeInterval::asSecond() const {
	const timeval& tv = reinterpret_cast<const timeval&>(mTicks);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}

#endif	// _WIN32

TimeInterval TimeInterval::getMax() {
	return TimeInterval(uint64_t(ULLONG_MAX));
}

Timer::Timer() {
	reset();
}

TimeInterval Timer::get() const {
	return TimeInterval(getTimeSinceMachineStartup() - mStartTime.asTicks());
}

TimeInterval Timer::reset()
{
	TimeInterval backup = get();
	mStartTime = TimeInterval(getTimeSinceMachineStartup());
	return backup;
}

DeltaTimer::DeltaTimer() {
	mLastTime = mTimer.get();
}

DeltaTimer::DeltaTimer(const TimeInterval& firstDelta) {
	mLastTime = mTimer.get() - firstDelta;
}

TimeInterval DeltaTimer::getDelta() const
{
	TimeInterval currentTime = mTimer.get();
	// We knows the minus operation won't produce negative result, so
	// use the ticks directly as an optimization
	uint64_t dt = currentTime.asTicks() - mLastTime.asTicks();
	mLastTime = currentTime;
	return TimeInterval(dt);
}

}	// namespace SGE
