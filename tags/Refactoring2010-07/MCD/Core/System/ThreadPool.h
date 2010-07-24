#ifndef __MCD_CORE_SYSTEM_THREADPOOL__
#define __MCD_CORE_SYSTEM_THREADPOOL__

#include "Thread.h"

namespace MCD {

/*!	A collection of threads that work for the same instance of Thread::IRunnable.
	\code
	class Runnable : public ThreadPool::IRunnable {
	public:
		sal_override void run(Thread& thread) throw() {
			while(thread.keepRun()) {
				// Do something useful
			}
		}
	};	// Runnable

	// ...

	Runnable runnable;
	ThreadPool threadPool(runnable, false);	// Tells threadPool not to delete runnable, since it's on the stack
	// Use 2 threads to execute runnable.run()
	threadPool.setThreadCount(2);
	// ThreadPool will wait until all thread finished before it's destruction
	\endcode
 */
class MCD_CORE_API ThreadPool : Noncopyable
{
public:
	/*!	The IRunnable interface.
		\sa Thread
		\sa Thread::IRunnable
	 */
	typedef Thread::IRunnable IRunnable;

	/*!	Construct the thread and associate it with an IRunnable.
		It will not create any thread until you call setThreadCount() with a positive number.
	 */
	explicit ThreadPool(IRunnable& runnable, bool deleteRunnableOnDestroy=true);

	//! The destructor will wait for all those threads to finish.
	~ThreadPool();

	/*! Set the number of thread.
		\param wait Weather the function should block until the desired number of thread is achieved.
	 */
	void setThreadCount(size_t targetCount, bool wait=false);

	size_t getThreadCount() const;

	IRunnable& runnable() const {
		return mRunnable;
	}

protected:
	class ThreadList;		//! Container that stores the threads
	class RunnableThread;	//! Thread, runnable and LinkListBase::NodeBase combined together so that we can put it in ThreadList

	ThreadList* mThreadList;
	mutable IRunnable& mRunnable;
	bool mDeleteRunnableOnDestroy;
};	// ThreadPool

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_THREADPOOL__
