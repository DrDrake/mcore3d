#include "Pch.h"
#include "../../../SGE/Core/System/ThreadPool.h"

using namespace SGE;

namespace {

//! Keep active until the thread inform it to quit
class Runnable : public SGE::Thread::IRunnable
{
public:
	sal_override void run(Thread& thread) throw()
	{
		while(thread.keepRun()) {
			mSleep(0);
		}
	}
};	// Runnable

}	// namespace

TEST(Basic_ThreadPoolTest)
{
	Runnable runnable;
	ThreadPool threadPool(runnable, false);
	threadPool.setThreadCount(1);
	mSleep(1);
	threadPool.setThreadCount(3);
	mSleep(1);
	threadPool.setThreadCount(2);
	mSleep(1);

	CHECK(true);
	// ThreadPool will wait untill all thread finished before it's destruction
}

TEST(SetCountWait_ThreadPoolTest)
{
	Runnable runnable;
	ThreadPool threadPool(runnable, false);
	threadPool.setThreadCount(5);

	// Calling setThreadCount twice, one without wait and one with wait
	threadPool.setThreadCount(0, false);
	threadPool.setThreadCount(0, true);

	CHECK(true);
}
