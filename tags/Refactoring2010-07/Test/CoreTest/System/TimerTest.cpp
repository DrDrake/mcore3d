#include "Pch.h"
#include "../../../MCD/Core/System/Timer.h"

using namespace MCD;

TEST(TimeIntervalTest)
{
	{	// Defautl construct as zero
		TimeInterval t;
		CHECK_EQUAL(0u, t.asTicks());
		CHECK_CLOSE(0, t.asSecond(), 1e-10);
	}

	{	// Construct using ticks
		TimeInterval t(uint64_t(1));
		CHECK_EQUAL(1u, t.asTicks());
		CHECK(t.asSecond() > 0);
	}

	{	// Consturct using second
		TimeInterval t(1.23);
		CHECK_CLOSE(1.23, t.asSecond(), 1e-6);
		CHECK(t.asTicks() > 0);
	}

	{	TimeInterval t;
		t.set(uint64_t(1));
		CHECK_EQUAL(1u, t.asTicks());

		t.set(1.23);
		CHECK_CLOSE(1.23, t.asSecond(), 1e-6);

		CHECK(t < TimeInterval::getMax());
	}

	{	// Operators
		TimeInterval t1, t2, t3(1.23);
		CHECK(t1 == t2);
		CHECK(t1 < t3);
		CHECK(!(t1 < t2));

		CHECK(t3 + t3 == t3 * 2);

		t1 += t3;
		CHECK(t3 - t1 == t2);
		t1 -= t3;
		CHECK(t1 == t2);
	}
}

TEST(TimerTest)
{
	Timer t;
	CHECK(t.get().asSecond() >= 0.0);
	t.reset();
	CHECK(t.get().asSecond() >= 0.0);

	// Check the time value is increasing
	static const size_t cTestCount = 10;
	double last = 0.0;
	for(size_t i=0; i<cTestCount; ++i) {
		double current = t.get().asSecond();
		CHECK(current >= last);
		last = current;
	}
}

TEST(DeltaTimerTest)
{
	{	DeltaTimer t;
		CHECK(t.getDelta().asSecond() >= 0);
	}

	{	DeltaTimer t(TimeInterval(1.0));
		CHECK(t.getDelta().asSecond() > 1.0 - 1e6);
	}
}
