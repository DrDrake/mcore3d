#include "Pch.h"
#include "TaskPool.h"
#include "CondVar.h"
#include "ThreadPool.h"
#include <memory>	// for std::auto_ptr

namespace MCD {

class TaskPool::TaskQueue : public Map<TaskPool::Task>
{
	typedef Map<TaskPool::Task> Super;

public:
	void insert(Task& task)
	{
		ScopeLock lock(mCondVar);
		Super::insert(task);
		mCondVar.signalNoLock();
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

		Task* task = Super::findMin();
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
	TaskQueue* queue = static_cast<TaskQueue*>(getMap());
	if(queue) {
		ScopeLock lock(queue->mCondVar);
		removeThis();
	}
}

int TaskPool::Task::priority() const
{
	return getKey();
}

class TaskPool::Runnable : public ThreadPool::IRunnable
{
public:
	Runnable(TaskQueue& queue) : mQueue(queue) {}

	void run(Thread& thread) throw()
	{
		while(thread.keepRun())
		{
			// Pick up a task, which also remove from the queue immediately
			Task* task = mQueue.pop(thread);

			// Run the task
			if(task)
				task->run(thread);
		}
	}

	TaskQueue& mQueue;
};	// Runnable

TaskPool::TaskPool()
{
	std::auto_ptr<TaskQueue> queue(new TaskQueue());
	mTaskQueue = queue.get();
	std::auto_ptr<Runnable> runnable(new Runnable(*mTaskQueue));
	mThreadPool = new ThreadPool(*runnable, true);
	queue.release();
	runnable.release();
}

TaskPool::~TaskPool()
{
	stop();

	// Must delete mThreadPool before mTaskQueue
	delete mThreadPool;
	delete mTaskQueue;
}

void TaskPool::enqueue(Task& task)
{
	mTaskQueue->insert(task);
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
	Task* task;
	while((task = mTaskQueue->pop(dummyThread)) != nullptr)
		task->run(dummyThread);
}

}	// namespace MCD
