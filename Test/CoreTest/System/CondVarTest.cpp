#include "Pch.h"
#include "../../../MCD/Core/System/CondVar.h"
#include "../../../MCD/Core/System/Thread.h"
#include "../../../MCD/Core/System/Timer.h"

using namespace MCD;

TEST(Basic_CondVarTest)
{
	class Runnable : public Thread::IRunnable
	{
	public:
		Runnable() : mFlag(false) {}
		void waitTillFlagIsTrue()
		{
			// When this loop exist, we sure mCondVar is in a wait state
			while(true) {
				{	ScopeLock lock(mCondVar);
					if(mFlag == true)
						break;
				}
				mSleep(0);
			}
		}
		sal_override void run(Thread& thread) throw()
		{
			// Rather than using mCondVar.wait() directly, we use lock() and unlock() so setting mFlag = true
			// becomes part of the atomic operation too.
			mCondVar.lock();
			mFlag = true;
			mCondVar.waitNoLock();
			mCondVar.unlock();
			(void)thread;
		}
		CondVar mCondVar;
		volatile bool mFlag;
	};

	{	Runnable runnable;
		Thread thread(runnable, false);
		runnable.waitTillFlagIsTrue();
		runnable.mCondVar.signal();
		thread.wait();
	}

	{	Runnable runnable;
		Thread thread(runnable, false);
		runnable.waitTillFlagIsTrue();
		runnable.mCondVar.broadcast();
		thread.wait();
	}

	{	// The wait should timeout
		CondVar condVar;
		CHECK(!condVar.wait(TimeInterval(0.001)));
	}
}

#include <deque>

namespace {

std::deque<int> gTestQueue;
Mutex gTestMutex;
CondVar gConsumerCondVar;
CondVar gProducerCondVar;

static const bool print = false;

class Producer : public Thread::IRunnable
{
public:
	Producer() : mCount(0) {}
	sal_override void run(Thread& thread) throw()
	{
		while(thread.keepRun()) {
			ScopeLock lock(gTestMutex);

			// Wait for the consumer to pop some of the elements
			while(gTestQueue.size() > 10) {
				gTestMutex.unlock();
				gConsumerCondVar.wait();
				gTestMutex.lock();
			}

			gTestQueue.push_back(++mCount);

			// Consumer can now fetch from the queue
			gProducerCondVar.signal();
		}
	}
	size_t mCount;
};	// Producer

class Consumer : public Thread::IRunnable
{
public:
	Consumer() : mCount(0) {}
	sal_override void run(Thread& thread) throw()
	{
		while(true) {
			ScopeLock lock(gTestMutex);

			// Wait for the producer to push more
			while(gTestQueue.empty()) {
				if(!thread.keepRun())
					return;

				gTestMutex.unlock();
				gProducerCondVar.wait();
				gTestMutex.lock();
			}

			MCD_ASSERT(!gTestQueue.empty());
			gTestQueue.pop_front();
			++mCount;

			// The queue is empty, producer can continue to push more
			if(gTestQueue.empty())
				gConsumerCondVar.signal();
		}
	}
	size_t mCount;
};	// Producer

}	// namespace

TEST(ProducerConsumer_CondVarTest)
{
	// TODO: Deadlock will happen in Cygwin
	Producer producer;
	Consumer consumer;
	Thread producerThread(producer, false);
	Thread consumerThread(consumer, false);

	// Make sure the producer and consumer have do something before the test finish
	gProducerCondVar.wait();

	mSleep(10);

	// We have to call PostQuit before signaling the condition variable
	producerThread.postQuit();
	gConsumerCondVar.signal();
	producerThread.wait();

	consumerThread.postQuit();
	gProducerCondVar.signal();
	consumerThread.wait();

	CHECK(producer.mCount > 0);
	CHECK(producer.mCount == consumer.mCount);
}
