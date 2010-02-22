#include "Pch.h"
#include "../../../MCD/Core/System/TaskPool.h"

using namespace MCD;

TEST(Basic_TaskPoolTest)
{
	class Task : public MCD::TaskPool::Task
	{
	public:
		Task(size_t loopCount=100, uint priority=0)
			:
			MCD::TaskPool::Task(priority),
			mLoopCount(loopCount),
			mSeed(0)
		{
		}

		sal_override void run(Thread& thread) throw()
		{
			MCD_ASSERT(taskPool);

			int ranNum = mSeed;

			for(size_t i=0; i<mLoopCount; ++i) {
				if(!thread.keepRun())
					return;
				ranNum = (123 * ranNum + 456) % 987654321;
			}

			mSeed = ranNum;
		}

		size_t mLoopCount;
		int mSeed;
	};	// Task

	{	// Simply create the task pool and then destroy it
		TaskPool taskPool;
		taskPool.setThreadCount(2);
	}

	{	// Create the pool and run some tasks
		Task task[10];
		TaskPool taskPool;
		taskPool.setThreadCount(2);
		CHECK_EQUAL(2u, taskPool.getThreadCount());

		for(size_t i=0; i<10; ++i) {
			CHECK_EQUAL(0, task[0].priority());
			taskPool.enqueue(task[i]);
			CHECK_EQUAL(&taskPool, task[i].taskPool);
		}

		mSleep(1);
	}
}

TEST(DeleteTask_TaskPoolTest)
{
	class Task : public MCD::TaskPool::Task
	{
	public:
		Task() : MCD::TaskPool::Task(0) {}

		sal_override void run(Thread&) throw() {
			delete this;
		}
	};	// Task

	TaskPool taskPool;
	taskPool.setThreadCount(2);

	for(size_t i=0; i<10; ++i)
		taskPool.enqueue(*(new Task));

	mSleep(1);
	CHECK(true);
}
