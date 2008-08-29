#include "Pch.h"
#include "ThreadPool.h"
#include "CondVar.h"
#include "LinkList.h"
#include <memory>

namespace MCD {

class ThreadPool::ThreadList : public LinkList<RunnableThread>
{
public:
	CondVar mCondVar;
};	// ThreadPool::ThreadList

class ThreadPool::RunnableThread : public LinkListBase::NodeBase, public Thread, public Thread::IRunnable
{
public:
	RunnableThread(ThreadPool& threadPool)
		:
		Thread(),
		mThreadPool(threadPool)
	{
		Thread::start(*this, false);
	}

	sal_override void run(Thread& thread) throw()
	{
		MCD_ASSERT(&thread == this);
		mThreadPool.mRunnable.run(thread);

		// The job is finished, destroy this thread
		{	MCD_ASSUME(mThreadPool.mThreadList != nullptr);

			CondVar& condVar = mThreadPool.mThreadList->mCondVar;
			ScopeLock lock(condVar);
			this->destroyThis();	// Will trigger the removal from the thread list and destruction of this runnable
			// This MyRunnable will also be destroyed after myThread is destroyed
			// Don't access 'this' pointer any more

			condVar.signalNoLock();
		}
	}

	ThreadPool& mThreadPool;
};	// RunnableThread

ThreadPool::ThreadPool(IRunnable& runnable, bool deleteRunnableOnDestroy)
	:
	mThreadList(new ThreadList()),
	mRunnable(runnable),
	mDeleteRunnableOnDestroy(deleteRunnableOnDestroy)
{
}

ThreadPool::~ThreadPool()
{
	// Make sure all thread finished their job before we going to destroy the thread list
	setThreadCount(0, true);
	delete mThreadList;

	if(mDeleteRunnableOnDestroy)
		delete &mRunnable;
}

void ThreadPool::setThreadCount(size_t targetCount, bool wait)
{
	MCD_ASSUME(mThreadList != nullptr);

	for(;;) {
		CondVar& condVar = mThreadList->mCondVar;
		ScopeLock lock(condVar);

		int delta = static_cast<int>(targetCount - mThreadList->elementCount());

		if(delta > 0) {	// Not enough, need more
			RunnableThread* myThread(new RunnableThread(*this));
			mThreadList->pushBack(*myThread);
		} else if(delta < 0) {	// Too much thread, quit some
			if(wait) {
				// Wait for a thread to quit
				mThreadList->front().postQuit();
				condVar.waitNoLock();
			} else {
				// Just simply call "delta" amount of threads to quit but no wait
				RunnableThread* t = &mThreadList->front();
				do {
					MCD_ASSUME(t != nullptr);
					t->postQuit();
					t = static_cast<RunnableThread*>(t->next());
				} while(++delta < 0);
				return;
			}
		} else {	// We got the desired number of thread, so return
			return;
		}
	}
}

size_t ThreadPool::getThreadCount() const
{
	MCD_ASSUME(mThreadList != nullptr);
	ScopeLock lock(mThreadList->mCondVar);
	return mThreadList->elementCount();
}

}	// namespace MCD
