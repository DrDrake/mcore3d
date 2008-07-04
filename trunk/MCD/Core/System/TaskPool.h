#ifndef __SGE_CORE_SYSTEM_TASKPOOL__
#define __SGE_CORE_SYSTEM_TASKPOOL__

#include "Thread.h"
#include "Map.h"

namespace SGE {

class ThreadPool;

/*!	Execute a prioritized list of tasks with a thread pool.

	Example:
	\code
	class Task : public SGE::TaskPool::Task {
	public:
		Task(int priority=0) : SGE::TaskPool::Task(priority) {}

		sal_override void run(Thread& thread) throw() {
			// Do something usefull
			// You may poll thread.keepRun() to decide weather you should abort your task or not
			// ...

			// The Task instance is not needed any more, destroy it.
			delete this;
		}
	};	// Task

	Task task[10];
	TaskPool taskPool;
	taskPool.setThreadCount(2);

	for(size_t i=0; i<10; ++i)
		taskPool.enqueue(*(new Task));

	// The tasks are now running in parallel with the current thread

	// All taskes submitted will have a chance to execute the run() function
	// when TaskPool is being deleted
	\endcode
 */
class SGE_CORE_API TaskPool : Noncopyable
{
public:
	/*!	A unit of task to put into the job pool.
		The task must handle their life time, and make sure it's destruction
		will not happen during the execution of run().
	 */
	class SGE_CORE_API SGE_NOVTABLE Task : public Thread::IRunnable, public MapBase<int>::Node<Task>
	{
	protected:
		Task(int priority);
		sal_override ~Task();

	public:
		/*!	Get the priority of the task
			{ Negative -> low, Normal -> normal, Positive -> hight } priority
		 */
		int priority() const;
	};	// Task

	TaskPool();

	/*!	The destructor will wait for all those tasks to finish.
		It make sure all tasks will execute it's Task::run() function, so that
		user have a chance to do cleanup before exiting run().
	 */
	~TaskPool();

	//! Put a task to the pool.
	void enqueue(Task& task);

	/*! Set the number of thread.
		\param wait Weather the function should block until the desired number of thread is achieved.
	 */
	void setThreadCount(size_t targetCount, bool wait=false);

	size_t getThreadCount() const;

protected:
	class TaskQueue;	//! Stores the task sorted by priority
	class Runnable;

	TaskQueue* mTaskQueue;
	ThreadPool* mThreadPool;
};	// TaskPool

}	// namespace SGE

#endif	// __SGE_CORE_SYSTEM_TASKPOOL__
