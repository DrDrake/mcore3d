#include "Pch.h"
#include "../../../MCD/Core/System/Thread.h"
#include <memory>	// For auto_ptr

using namespace MCD;

namespace {

class MyRunnable : public MCD::Thread::IRunnable
{
public:
	MyRunnable() : valid(false) {}

protected:
	sal_override void run(Thread& thread) {
		(void)thread;
		valid = true;
	}

public:
	bool valid;
};	// MyRunnable

//! Keep active until the thread inform it to quit
class LoopRunnable : public MCD::Thread::IRunnable
{
public:
	LoopRunnable() : LoopCount(0) {}

protected:
	sal_override void run(Thread& thread)
	{
		while(thread.keepRun()) {
			mSleep(0);
			++LoopCount;
		}
	}

private:
	size_t LoopCount;
};	// LoopRunnable

}	// namespace

TEST(Basic_ThreadTest)
{
	{	// Construct Thread without start
		Thread thread;
		CHECK(!thread.isWaitable());
		CHECK_EQUAL(0, thread.id());
	}

	{	// Thread creation and wait
		MyRunnable runnable;
		Thread thread(runnable, false);
		CHECK_EQUAL(&runnable, thread.runnable());
		CHECK(thread.isWaitable());
		CHECK(thread.id() != 0);
		CHECK(thread.id() != getCurrentThreadId());
		thread.wait();
		CHECK(!thread.isWaitable());
		CHECK_EQUAL(0, thread.id());
		CHECK(runnable.valid);
	}

	{	// Multiple call to start
		MyRunnable runnable;
		Thread thread(runnable, false);
		CHECK(thread.keepRun());
		CHECK(thread.isWaitable());
		thread.wait();
		CHECK(!thread.keepRun());
		CHECK(!thread.isWaitable());

		// Second start
		thread.start(runnable, false);
		CHECK(thread.keepRun());
		CHECK(thread.isWaitable());
	}

	{	// Thread creation and auto delete of runnable
		MyRunnable* runnable = new MyRunnable;
		Thread thread(*runnable, true);
		thread.wait();
		CHECK(runnable->valid);
	}

	{	// Multiple call to start with different instance of runnable
		MyRunnable* runnable = new MyRunnable;
		Thread thread(*runnable, true);
		thread.wait();
		CHECK(runnable->valid);

		{	MyRunnable runnable2;
			thread.start(runnable2, false);
			thread.wait();
		}

		runnable = new MyRunnable;
		thread.start(*runnable, true);
		thread.wait();

		{	MyRunnable runnable2;
			thread.start(runnable2, false);
			thread.wait();
		}
	}

	// Get/Set priority only work correctly on win32
	// Totally not working for CYGWIN, only works for super user on Linux
#ifdef MCD_WIN
	{	// Test for get/set priority
		LoopRunnable runnable;
		Thread thread(runnable, false);

		CHECK_EQUAL(Thread::NormalPriority, thread.getPriority());

		thread.setPriority(Thread::LowPriority);
		CHECK_EQUAL(Thread::LowPriority, thread.getPriority());

		thread.setPriority(Thread::HighPriority);
		CHECK_EQUAL(Thread::HighPriority, thread.getPriority());

		thread.setPriority(Thread::NormalPriority);
		CHECK_EQUAL(Thread::NormalPriority, thread.getPriority());
	}
#endif	// MCD_WIN
}

#include "../../../MCD/Core/System/CondVar.h"

TEST(DeleteThread_ThreadTest)
{
	// The thread function will try to destroy the thread itself
	class DeleteThreadRunnable : public MCD::Thread::IRunnable
	{
	public:
		DeleteThreadRunnable() : mFinished(false) {}

	protected:
		sal_override void run(Thread& thread)
		{
			delete &thread;
			mFinished = true;
			mCondVar.signal();
		}

	public:
		AtomicValue<bool> mFinished;
		CondVar mCondVar;
	};	// DeleteThreadRunnable

	DeleteThreadRunnable runnable;

	// Never try to reference the thread anymore since it may be deleted at any time
	Thread* thread = new Thread(runnable, false);
	thread = nullptr;

	// We cannot use thread->wait(), use a boolean variable and condition variable instead.
	// The thread object should be destroyed after the wait
	while(!runnable.mFinished)
		runnable.mCondVar.wait();

	CHECK(true);
}

// NOTE: Exception support is removed from MCore, those related test cases
// will cause an assertion instead
#undef CHECK_THROW
#define CHECK_THROW(x, y) {}

TEST(Negative_ThreadTest)
{
	{	// Without start
		Thread thread;
		CHECK_THROW(thread.wait(), std::logic_error);
		CHECK_THROW(thread.setPriority(Thread::NormalPriority), std::logic_error);
		CHECK_THROW(thread.getPriority(), std::logic_error);
	}

	{	// Double start
		LoopRunnable runnable;
		Thread thread(runnable, false);
		CHECK_THROW(thread.start(runnable, false), std::logic_error);
	}

	{	// Double wait
		LoopRunnable runnable;
		Thread thread(runnable, false);
		thread.wait();
		CHECK_THROW(thread.wait(), std::logic_error);
		CHECK_THROW(thread.setPriority(Thread::NormalPriority), std::logic_error);
		CHECK_THROW(thread.getPriority(), std::logic_error);
	}
}

TEST(TryLock_ThreadTest)
{
	class LockMutexRunnable : public MCD::Thread::IRunnable
	{
	public:
		LockMutexRunnable(Mutex& mutexToLock)
			: mMutexToLock(mutexToLock) {}

	protected:
		sal_override void run(Thread& thread) {
			ScopeLock lock(mMutexToLock);
			while(thread.keepRun()) {}
		}

	public:
		Mutex& mMutexToLock;
	};	// MyRunnable

	{	Mutex mutex;
		LockMutexRunnable runnable(mutex);
		Thread thread(runnable, false);
		// Loop until tryLock fail
		while(mutex.tryLock())
			mutex.unlock();
		CHECK(true);
	}

	{	Mutex mutex;
		ScopeUnlockOnly unlock(mutex);
		CHECK(mutex.tryLock());
	}

	{	RecursiveMutex mutex;
		ScopeRecursiveUnlockOnly unlock(mutex);
		CHECK(mutex.tryLock());
	}
}

#include "../../../MCD/Core/System/Deque.h"

namespace {

std::deque<int> gTestQueue;
Mutex gTestMutex;

class Producer : public Thread::IRunnable
{
	sal_override void run(Thread& thread)
	{
		while(thread.keepRun()) {
			ScopeLock lock(gTestMutex);
			gTestQueue.push_back(1);
		}
	}
};	// Producer

class Consumer : public Thread::IRunnable
{
	sal_override void run(Thread& thread)
	{
		for(;;) {
			ScopeLock lock(gTestMutex);
			if(!gTestQueue.empty())
				gTestQueue.pop_front();
			else if(!thread.keepRun())
				break;
		}
	}
};	// Producer

}	// namespace

TEST(ProducerConsumer_ThreadTest)
{
	Producer producer;
	Consumer consumer;
	Thread producerThread(producer, false);
	Thread consumerThread(consumer, false);
	mSleep(1);
	uSleep(1);
	producerThread.wait();
	consumerThread.wait();
	CHECK(true);
}
