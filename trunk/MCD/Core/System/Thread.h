#ifndef __MCD_CORE_SYSTEM_THREAD__
#define __MCD_CORE_SYSTEM_THREAD__

#include "Atomic.h"

namespace MCD {

/*!	A thread of execution in a program.
	User need to submit an object of Thread::IRunnable in order for Thread to execute.

	\note Suspend and Resume is removed, since user can use CondVar to do the same.
		Using Suspend and Resume is dangerous, since you don't know where the thread
		stops if you invoke Suspend in another thread.

	\code
	class MyRunnable : public MCD::Thread::IRunnable {
	public:
		MyRunnable() : LoopCount(0) {}
		// Keep printing hello world until the thread wants to stop
		sal_override void run(Thread& thread) {
			while(thread.keepRun())
				std::cout << "Hello world!\n";
		}
	};	// MyRunnable

	// ...

	MyRunnable runnable;
	Thread thread(runnable, false);	// Tells thread not to delete runnable, since it's on the stack
	MCD::mSleep(1000);	// Sleep for 1 seconds
	thread.wait();	// Tells the thread we are going to stop and then wait until MyRunnable::run() returns
	\endcode
 */
class MCD_CORE_API Thread : Noncopyable
{
public:
	//! Sub-classing IRunnable to do the real work for Thread.
	class MCD_ABSTRACT_CLASS IRunnable
	{
	public:
		virtual ~IRunnable() {}

		/*! Override this method to do the actual work.
			\note This method may invoked by a number of different threads since different
				Thread can construct with the same IRunnable instance. Therefore please pay
				attention to thread safety on this method.
			\param thread Represent the thread context which invoke this run() method
		 */
		virtual void run(Thread& thread) = 0;
	};	// IRunnable

	//! Construct a Thread instance but without any thread created.
	Thread();

	/*!	Construct a Thread and start it with an IRunnable.
		\sa start()
	 */
	Thread(IRunnable& runnable, bool autoDeleteRunnable=true);

	//! The destructor will wait for the thread to finish.
	~Thread();

	/*!	Associate the thread with an IRunnable and start it.
		\param runnable Make sure it will \em not be deleted before the thread function finish.
		\param autoDeleteRunnable Indicate Thread should manage the life-time of the runnable or not.
		\note Thread creation is submitted to the under laying operation system inside
			the constructor, it didn't guarantee the IRunnable::run() function get
			started before the constructor exit.
	 */
	void start(IRunnable& runnable, bool autoDeleteRunnable=true);

	sal_maybenull IRunnable* runnable() const {
		return mRunnable;
	}

	/*! Set a flag so that keepRun() will return false.
		This function is used to inform the IRunnable to quit the loop as soon as possible.
	 */
	void postQuit();

	//! Return false if postQuit() is invoked.
	bool keepRun() const;

	void setKeepRun(bool b);

	/*! Wait until the runnable finish.
		\note It will invoke postQuit()
		\note Exception if it's not in a waitable state (eg deadlock if it's called inside IRunnable::run())
	 */
	void wait();

	enum Priority
	{
		LowPriority = -1,
		NormalPriority = 0,
		HighPriority = 1
	};	// Priority

	/*!	Get the thread's scheduling priority.
		\note Not working on CYGWIN, only work for super user on Linux
	 */
	Priority getPriority() const;

	/*!	Set the thread's scheduling priority.
		Only effective after the thread is started.
		\note Not working on CYGWIN, only work for super user on Linux
	 */
	void setPriority(Priority priority);

	//! Get this thread's ID
	int id() const;

	//! Check that calling wait() will not causing error or deadlock.
	bool isWaitable() const;

protected:
	void init();
	void cleanup();

protected:
	IRunnable* mRunnable;
#ifdef MCD_WIN32
	intptr_t mHandle;
	int mId;
#else
	pthread_t mHandle;
#endif
	Priority mPriority;
	bool mAutoDeleteRunnable;
	AtomicValue<bool> mKeepRun;
	mutable RecursiveMutex mMutex;	//! To protect member like mHandle and mId against race-conditions.
};	// Thread

MCD_CORE_API int getCurrentThreadId();

//! Mill-second sleep.
MCD_CORE_API void mSleep(size_t millseconds);

//! Portable Unix usleep.
MCD_CORE_API void uSleep(useconds_t microseconds);

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_THREAD__
