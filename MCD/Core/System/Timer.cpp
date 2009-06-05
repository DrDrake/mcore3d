#include "Pch.h"
#include "Timer.h"
#include "PlatformInclude.h"
#include "StaticAssert.h"
#include <limits.h>	// For ULLONG_MAX
#include <math.h>	// For floof

#define USE_RDTSC 1

namespace MCD {

#if USE_RDTSC
#	ifdef MCD_VC
#		define rdtsc(low,high)	\
		__asm rdtsc				\
		__asm mov low, eax		\
		__asm mov high, edx
#	elif defined(MCD_GCC)
#		define rdtsc(low,high)	\
		__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#	else
#	endif
#endif

union Counter64 {
	uint32_t n32[2];
	uint64_t n64;
};

uint64_t getTimeSinceMachineStartup()
{
	uint64_t ret;

#if USE_RDTSC
	uint32_t l, h;
	Counter64 tmp;
	rdtsc(l, h);
	tmp.n32[0] = l;
	tmp.n32[1] = h;
	ret = tmp.n64;
#elif defined(_WIN32)
	::QueryPerformanceCounter((LARGE_INTEGER*)(&ret));
#else
	timeval tv;
	::gettimeofday(&tv, nullptr);
	ret = reinterpret_cast<const uint64_t&>(tv);
#endif

	return ret;
}

#ifdef _WIN32

uint64_t getQueryPerformanceFrequency()
{
	LARGE_INTEGER ret;
	BOOL ok = ::QueryPerformanceFrequency(&ret);
	(void)ok;
	MCD_ASSERT(ok && "QueryPerformanceFrequency failed");
	MCD_STATIC_ASSERT(sizeof(uint64_t) == sizeof(ret.QuadPart));
#if USE_RDTSC
	// Try to get the ratio between QueryPerformanceCounter and rdtsc
	// TODO: Rdtsc is still not consider as a very robust time measurement method,
	// it will be better to define another class of timer that seperate the timer
	// from general usage and performance profiling usage.
	uint64_t ticks1;
	{	::QueryPerformanceCounter((LARGE_INTEGER*)(&ticks1));
	}
	uint64_t ticks2;
	{	uint32_t l, h;
		Counter64 tmp;
		rdtsc(l, h);
		tmp.n32[0] = l;
		tmp.n32[1] = h;
		ticks2 = tmp.n64;
	}

	double ratio = double(ticks2) / ticks1;
	return uint64_t(ret.QuadPart * ratio);
#else
	return uint64_t(ret.QuadPart);
#endif
}

static uint64_t cTicksPerSecond = getQueryPerformanceFrequency();
static double cInvTicksPerSecond = 1.0 / cTicksPerSecond;

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

}	// namespace MCD
