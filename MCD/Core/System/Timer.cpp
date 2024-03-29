#include "Pch.h"
#include "Timer.h"
#include "PlatformInclude.h"
#include "StaticAssert.h"
#include <limits.h>	// For ULLONG_MAX
#include <math.h>	// For floof

#ifdef MCD_APPLE
#	include <mach/mach_time.h>
#elif defined(MCD_WIN32)
#	define USE_RDTSC 1
#endif

namespace MCD {

#if USE_RDTSC
#	ifdef MCD_VC
#		define RDTSC(low, high)	\
		__asm rdtsc				\
		__asm mov low, eax		\
		__asm mov high, edx
#	elif defined(MCD_GCC)
#		define RDTSC(low, high)	\
		__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#	else
#	endif
#endif

// mftbu in PowerPC: http://lists.apple.com/archives/mac-games-dev/2002/May/msg00244.html
#if USE_RDTSC
inline uint64_t rdtsc() {
	uint32_t l, h;
	RDTSC(l, h);
	return (uint64_t(h) << 32) + l;
}
#endif

uint64_t ticksSinceMachineStartup()
{
	uint64_t ret;

#if USE_RDTSC
	ret = rdtsc();
#elif defined(_WIN32)
	::QueryPerformanceCounter((LARGE_INTEGER*)(&ret));
#elif defined(MCD_APPLE)
	ret = mach_absolute_time();
#else
	timeval tv;
	::gettimeofday(&tv, nullptr);
	ret = reinterpret_cast<const uint64_t&>(tv);
#endif

	return ret;
}

#if defined(_WIN32) || defined(MCD_APPLE)

// Number of unit in one second.
uint64_t getQueryPerformanceFrequency()
{
#if USE_RDTSC
	LARGE_INTEGER ret;
	MCD_VERIFY(::QueryPerformanceFrequency(&ret));
	MCD_STATIC_ASSERT(sizeof(uint64_t) == sizeof(ret.QuadPart));

	// Try to get the ratio between QueryPerformanceCounter and rdtsc
	uint64_t ticks1;
	::QueryPerformanceCounter((LARGE_INTEGER*)(&ticks1));
	uint64_t ticks2 = rdtsc();

	// In most cases, the absolution values of QueryPerformanceCounter and rdtsc
	// can be used to computer their clock frequency ratio. But in some cases, for
	// example the machine is waken up from a sleeping state, the CPU clock is stoped
	// while the mother board's clock is still running, making them out of sync after
	// the machine wake up.
	double ratio = double(ticks2) / ticks1;
	if(true || ratio < 1) {	// Absolute value messed up, calcuate relative value.
		double dummy = double(ret.LowPart);
		for(int i=0; i<1000; ++i)
			dummy = sin(dummy);
		uint64_t ticks1b;
		::QueryPerformanceCounter((LARGE_INTEGER*)(&ticks1b));
		uint64_t ticks2b = rdtsc();
		ticks1 = ticks1b - ticks1;
		ticks2 = ticks2b - ticks2;
		// NOTE: The action of adding "dummy" to the equation is to
		// prevent the compiler optimize away the sin(dummy) operation.
		// And since the value of sin() always smaller than 1, so the error
		// introduced is minimum.
		ratio = double(ticks2 + dummy) / (ticks1);
	}

	return uint64_t(ret.QuadPart * ratio);
#elif defined(MCD_APPLE)
	// Reference: http://www.macresearch.org/tutorial_performance_and_time
	// Reference: http://developer.apple.com/mac/library/qa/qa2004/qa1398.html
	mach_timebase_info_data_t info;
	if(mach_timebase_info(&info) == 0)
		return uint64_t(1e9 * (double) info.denom / (double) info.numer);
	else
		return 0;
#else
	return 0;
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
	tv.tv_sec = uint32_t(::floor(sec));
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
	return TimeInterval(ticksSinceMachineStartup() - mStartTime.asTicks());
}

TimeInterval Timer::reset()
{
	TimeInterval backup = get();
	mStartTime = TimeInterval(ticksSinceMachineStartup());
	return backup;
}

static Timer gSinceProgramStatup;

TimeInterval Timer::sinceProgramStatup()
{
	return gSinceProgramStatup.get();
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
