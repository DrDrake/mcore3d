#ifndef __MCD_CORE_SYSTEM_TASKPOOL__
#define __MCD_CORE_SYSTEM_TASKPOOL__

#include "Thread.h"
#include "Map.h"

namespace MCD {

class ThreadPool;

/*!	Execute a prioritized list of tasks with a thread pool.

	Example:
	\code
	class Task : public MCD::TaskPool::Task {
	public:
		Task(int priority=0) : MCD::TaskPool::Task(priority) {}

		sal_override void run(Thread& thread) throw() {
			// Do something useful
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

	// All tasks submitted will have a chance to execute the run() function
	// when TaskPool is being deleted
	\endcode
 */
class MCD_CORE_API TaskPool : Noncopyable
{
public:
	/*!	A unit of task to put into the job pool.
		The task must handle their life time, and make sure it's destruction
		will not happen during the execution of run().

		\note We didn't store a pointer back to TaskPool, cos it's too complicated to 
			deal with both the life-time and thread problem (especially Task::update() may invoke
			delete this). This is better be done on application level where more assumption can be made.
	 */
	class MCD_CORE_API MCD_ABSTRACT_CLASS Task
		: public Thread::IRunnable
		, private MapBase<int>::Node<Task>	// The interface found it Map class should NOT expose to outside.
	{
	protected:
		friend class TaskPool;
		typedef MapBase<int>::Node<Task>::KeyArg KeyArg;
		template <typename T1, typename T2> friend class Map;

		explicit Task(int priority);
		sal_override ~Task();

	public:
		/*!	Get the priority of the task
			{ Negative -> low, Zero -> normal, Positive -> hight } priority
		 */
		int priority() const;

		// TODO: Not thread safe!
		void setPriority(int priority);
	};	// Task

	TaskPool();

	/*!	The destructor will wait for all those tasks to finish.
		It make sure all tasks will execute it's Task::run() function, so that
		user have a chance to do cleanup before exiting run().
		\note
			For some cases you may need to call stop() explicitly before ~TaskPool();
			For example, the tasks may reference some object that should be destroyed
			before the task pool.
	 */
	~TaskPool();

	/*!	Put a task to the pool. Fail if the task is already in a queue.
		\note Keep retry the enqueue operation if you need to ensure it success.
	 */
	sal_checkreturn bool enqueue(Task& task);

	//!	If the thread count of the task pool is zero, call this function to process all the tasks.
	void processTaskIfNoThread();

	/*! Set the number of thread.
		\param wait Weather the function should block until the desired number of thread is achieved.
		\note
			Calling setThreadCount(0, true) didn't means all the task are visited,
			use stop() for that purpose.
	 */
	void setThreadCount(size_t targetCount, bool wait=false);

	size_t getThreadCount() const;

	/*!	Stop the task pool.
		It will setThreadCount(0, true) and then call Task::run() of each task to give them a chance
		to do any necessary cleanup.
	 */
	void stop();

protected:
	class TaskQueue;	//! Stores the task sorted by priority
	class Runnable;

	TaskQueue* mTaskQueue;
	ThreadPool* mThreadPool;
};	// TaskPool

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_TASKPOOL__
