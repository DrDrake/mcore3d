#include "Pch.h"
#include "TaskPool.h"
#include "CondVar.h"
#include "ThreadPool.h"
#include "Timer.h"

#ifdef MCD_IPHONE
#	import <Foundation/NSAutoreleasePool.h>
#endif

namespace MCD {

class TaskPool::TaskQueue : public Map<TaskPool::Task>
{
	typedef Map<TaskPool::Task> Super;

public:
	sal_checkreturn bool insert(Task& task)
	{
		ScopeLock lock(mCondVar);
		if(task.isInMap())
			return false;
		Super::insert(task);
		mCondVar.signalNoLock();
		return true;
	}

	sal_checkreturn Task* pop(Thread& thread)
	{
		ScopeLock lock(mCondVar);

		// Block if there is no task to do
		while(Super::isEmpty()) {
			if(!thread.keepRun())
				break;
			mCondVar.waitNoLock();
		}

		// Process task with bigger priority number first
		Task* task = Super::findMax();
		if(task) {
			task->removeThis();
			mCondVar.signalNoLock();
		}

		return task;
	}

	void waitTillEmpty()
	{
		ScopeLock lock(mCondVar);
		while(!Super::isEmpty())
			mCondVar.waitNoLock();
	}

	CondVar mCondVar;
};	// TaskQueue

TaskPool::Task::Task(int priority)
	: MapBase<int>::Node<Task>(priority)
{
}

TaskPool::Task::~Task()
{
	// Remove the task from the queue before destruct
	if(TaskQueue* queue = static_cast<TaskQueue*>(getMap())) {
		ScopeLock lock(queue->mCondVar);
		removeThis();
	}
}

int TaskPool::Task::priority() const
{
	return getKey();
}

void TaskPool::Task::setPriority(int priority)
{
	return setKey(priority);
}

class TaskPool::Runnable : public ThreadPool::IRunnable
{
public:
	Runnable(TaskQueue& queue) : mQueue(queue) {}

	void run(Thread& thread)
	{
		while(thread.keepRun())
		{
			// Pick up a task, which also remove from the queue immediately
			Task* task = mQueue.pop(thread);

#ifdef MCD_IPHONE
			// Make sure every thread has a NSAutoreleasePool
			// Reference: http://www.idevgames.com/forum/archive/index.php/t-7710.html
			NSAutoreleasePool* autoreleasepool = [[NSAutoreleasePool alloc] init];
#endif

			// Run the task
			if(task)
				task->run(thread);

#ifdef MCD_IPHONE
			[autoreleasepool release];
#endif
		}
	}

	TaskQueue& mQueue;
};	// Runnable

TaskPool::TaskPool()
{
	mTaskQueue = new TaskQueue();
	Runnable* runnable(new Runnable(*mTaskQueue));
	mThreadPool = new ThreadPool(*runnable, true);
}

TaskPool::~TaskPool()
{
	stop();

	// Must delete mThreadPool before mTaskQueue
	delete mThreadPool;
	delete mTaskQueue;
}

bool TaskPool::enqueue(Task& task)
{
	return mTaskQueue->insert(task);
}

void TaskPool::processTaskInThisThread(Timer* timer, float timeOut)
{
	ScopeLock lock(mTaskQueue->mCondVar);

	Thread dummyThread;
	dummyThread.setKeepRun(true);

	while(Task* task = mTaskQueue->findMax()) {
		if(timer && float(timer->get().asSecond()) >= timeOut)
			break;

		task->removeThis();
		ScopeUnlock unlock(mTaskQueue->mCondVar);
		task->run(dummyThread);
	}
}

void TaskPool::setThreadCount(size_t targetCount, bool wait)
{
	MCD_ASSUME(mThreadPool != nullptr);

	// This is a common patten to inform the thread to quit, do a
	// condition variable broadcast and then wait for the threads to quit.
	if(wait) {
		mThreadPool->setThreadCount(targetCount, false);
		mTaskQueue->mCondVar.broadcast();
	}
	mThreadPool->setThreadCount(targetCount, wait);
}

size_t TaskPool::getThreadCount() const
{
	MCD_ASSUME(mThreadPool != nullptr);
	return mThreadPool->getThreadCount();
}

void TaskPool::stop()
{
	// Turn off all the threads first.
	TaskPool::setThreadCount(0, true);

	// Then run though all the task, so that there is a chance for those tasks
	// to clean up themself (eg. call "delete this;" in the run() function)
	Thread dummyThread;
	while(Task* task = mTaskQueue->pop(dummyThread))
		task->run(dummyThread);
}

}	// namespace MCD
