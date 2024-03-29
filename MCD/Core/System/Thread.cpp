#include "Pch.h"
#include "Thread.h"
#include "PlatformInclude.h"
#include "Utility.h"
#include <memory.h> // For memset

namespace MCD {

#ifdef MCD_WIN
DWORD WINAPI _Run(sal_notnull LPVOID p) {
#else
void* _Run(sal_notnull void* p) {
#endif
	Thread* t = reinterpret_cast<Thread*>(p);
	Thread::IRunnable* runnable = t->runnable();
	MCD_ASSUME(runnable != nullptr);
	runnable->run(*t);
	return 0;
}

Thread::Thread()
{
	init();
}

Thread::Thread(IRunnable& runnable, bool autoDeleteRunnable)
{
	init();
	start(runnable, autoDeleteRunnable);
}

void Thread::init()
{
	mRunnable = nullptr;
#ifdef MCD_WIN
	mHandle = nullptr;
	mId = 0;
#else
	mHandle = pthread_t(nullptr);
#endif
	mPriority = NormalPriority;
	mAutoDeleteRunnable = false;
	mKeepRun = false;
}

Thread::~Thread()
{
	// No need to lock

	if(isWaitable())
		wait();
	else
		cleanup();

	if(mAutoDeleteRunnable)
		delete mRunnable;
}

void Thread::start(IRunnable& runnable, bool autoDeleteRunnable)
{
	ScopeRecursiveLock lock(mMutex);

	MCD_ASSERT(!mHandle && "A thread is already in execution, call wait() before starting another one");

	// Delete the previous runnable if needed
	if(mRunnable && mAutoDeleteRunnable)
		delete mRunnable;

	mAutoDeleteRunnable = autoDeleteRunnable;
	mRunnable = &runnable;
	mKeepRun = true;

#ifdef MCD_WIN
	MCD_ASSERT(mId == 0);
	mHandle = reinterpret_cast<intptr_t>(::CreateThread(nullptr, 0, &_Run, this, 0, (LPDWORD)&mId));
	if(!mHandle)
#else
	if(::pthread_create(&mHandle, nullptr, &_Run, this) != 0)
#endif
		logSystemErrorMessage("Error creating thread.");
}

void Thread::postQuit()
{
	// No need to lock (mKeepRun is an AtomicValue)
	mKeepRun = false;
}

bool Thread::keepRun() const
{
	// No need to lock (mKeepRun is an AtomicValue)
	return mKeepRun;
}

void Thread::setKeepRun(bool b)
{
	// No need to lock (mKeepRun is an AtomicValue)
	mKeepRun = b;
}

bool Thread::isWaitable() const
{
	ScopeRecursiveLock lock(mMutex);
	return mHandle && MCD::getCurrentThreadId() != id();
}

void Thread::cleanup()
{
	ScopeRecursiveLock lock(mMutex);

	if(!mHandle)
		return;

#ifdef MCD_WIN
	MCD_VERIFY(::CloseHandle(reinterpret_cast<HANDLE>(mHandle)));
	mHandle = nullptr;

	MCD_ASSERT(mId != 0);
	mId = 0;
#else
	MCD_ASSERT(mHandle);
	::pthread_detach(mHandle);
	mHandle = pthread_t(nullptr);
#endif
}

Thread::Priority Thread::getPriority() const
{
	ScopeRecursiveLock lock(mMutex);
	MCD_ASSUME(mHandle);

#ifdef MCD_CYGWIN
	MCD_ASSERT(false && "Not implemented.");
#elif defined(MCD_WIN)
	int ret = ::GetThreadPriority(reinterpret_cast<HANDLE>(mHandle));
	if(ret == THREAD_PRIORITY_ERROR_RETURN)
		logSystemErrorMessage("Error getting thread priority.");

	if(ret > 0)
		ret = HighPriority;
	else if(ret < 0)
		ret = LowPriority;

	return Priority(ret);
#else
	sched_param param;
	int policy;
	MCD_VERIFY(::pthread_getschedparam(mHandle, &policy, &param) == 0);

	if(policy == SCHED_RR)
		return NormalPriority;
	else if(param.sched_priority > 50)
		return HighPriority;
	else
		return LowPriority;
#endif
}

void Thread::setPriority(Priority priority)
{
	ScopeRecursiveLock lock(mMutex);
	MCD_ASSUME(mHandle);

#ifdef MCD_CYGWIN
	(void)priority;
	MCD_ASSERT(false && "Not implemented.");
#elif defined(MCD_WIN)
	if(::SetThreadPriority(reinterpret_cast<HANDLE>(mHandle), int(priority)) == 0)
		logSystemErrorMessage("Error setting thread priority.");
#else
	// How to set thread priority on Linux:
	// http://cs.pub.ro/~apc/2003/resources/pthreads/uguide/users-31.htm
	// http://www.developertutorials.com/tutorials/linux/processes-threads-050616/page10.html
	sched_param param;
	::memset(&param, 0, sizeof(param));

	int policy = SCHED_RR;
	if(priority == NormalPriority)
		policy = SCHED_OTHER;
	else if(priority == LowPriority)
		param.sched_priority = 20;
	else if(priority == HighPriority)
		param.sched_priority = 70;
	else
		MCD_ASSUME(false);
	if(::pthread_setschedparam(mHandle, policy, &param) != 0)
		logSystemErrorMessage("Error setting thread priority.");
#endif
}

void Thread::wait()
{
	ScopeRecursiveLock lock(mMutex);

	MCD_ASSERT(isWaitable());

	postQuit();

	MCD_ASSERT(mHandle);

#ifdef MCD_WIN
	HANDLE handleBackup = reinterpret_cast<HANDLE>(mHandle);
	mHandle = nullptr;

	MCD_ASSERT(mId != 0);
	mId = 0;

	// Unlock the mutex before the actual wait operation
	lock.mutex().unlock();
	DWORD res = ::WaitForSingleObject(handleBackup, INFINITE);
	lock.mutex().lock();

	if(res == WAIT_FAILED)
		logSystemErrorMessage("Error waiting thread.");
	MCD_ASSERT(res == WAIT_OBJECT_0);

	MCD_VERIFY(::CloseHandle(handleBackup));
#else
	pthread_t handleBackup = mHandle;
	mHandle = pthread_t(nullptr);

	lock.mutex().unlock();
	lock.cancel();

	MCD_VERIFY(::pthread_join(handleBackup, nullptr) == 0);
#endif
}

int Thread::id() const
{
	ScopeRecursiveLock lock(mMutex);

#ifdef MCD_WIN
	return mId;
#else
	return int(mHandle);
#endif
}

int getCurrentThreadId()
{
#ifdef MCD_WIN
	return ::GetCurrentThreadId();
#else
	return int(::pthread_self());
#endif
}

void mSleep(size_t millseconds)
{
#ifdef MCD_WIN
	::Sleep(DWORD(millseconds));
#else
	::usleep(useconds_t(millseconds * 1000));
#endif
}

void uSleep(useconds_t microseconds)
{
#ifdef MCD_WIN
	::Sleep(microseconds / 1000);
#else
	::usleep(microseconds);
#endif
}

}	// namespace MCD
