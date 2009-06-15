#ifndef __MCD_CORE_SYSTEM_MUTEX__
#define __MCD_CORE_SYSTEM_MUTEX__

#include "../ShareLib.h"
#include "NonCopyable.h"

#if !defined(MCD_WIN32)
#	include <pthread.h>
#endif

namespace MCD {

//! Mutex.
class MCD_CORE_API Mutex : Noncopyable
{
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
	bool tryLock();

#ifndef NDEBUG
	//! For use in debug mode to assert that the mutex is locked.
	bool isLocked() { return _locked; }
#endif

#ifdef MCD_WIN32
	/*!	A char buffer that pretended to be a CRITICAL_SECTION.
		Using such an approach, we need not to include Windows.h
		The sizeof(CRITICAL_SECTION) is 24 on win32
	 */
	char mMutex[24];
#else
	pthread_mutex_t mMutex;
#endif

#ifndef NDEBUG
protected:
	bool _locked;
	char _padding[3];
#endif
};	// Mutex

//! RecursiveMutex
class MCD_CORE_API RecursiveMutex : private Noncopyable
{
public:
	RecursiveMutex();
	~RecursiveMutex();

	void lock();
	void unlock();
	bool tryLock();

#ifdef MCD_WIN32
	char mMutex[24];
#else
	pthread_mutex_t mMutex;
#endif
};	// RecursiveMutex

//! Common class for some cancel-able classes.
class Cancelable {
public:
	Cancelable() : mCanceled(false) {}
	void cancel() { mCanceled = true; }
	void resume() { mCanceled = false; }
	bool isCanceled() { return mCanceled; }

protected:
	bool mCanceled;
};	// RecursiveMutex

/*! Lock mutex in scope.
	Example:
	\code
	Mutex mutex;
	// ...
	{	ScopeLock lock(mutex);
		// We now protected by mutex, let's do something
		// ...
	}	// mutex get unlocked when out of scope
	\endcode
 */
class ScopeLock : public Cancelable, private Noncopyable
{
public:
	explicit ScopeLock(Mutex& m) : Cancelable(), m(m) { m.lock(); }
	~ScopeLock() { if(!isCanceled()) m.unlock(); }
	Mutex& m;
};	// ScopeLock


class ScopeUnlock : public Cancelable, private Noncopyable
{
public:
	explicit ScopeUnlock(Mutex& m) : Cancelable(), m(m) { m.unlock(); }
	~ScopeUnlock() { if(!isCanceled()) m.lock(); }
	Mutex& m;
};	// ScopeUnlock

/*! Unlocking mutex in scope.
	\note Make sure the mutex is locked before ScopeUnlockOnly try to unlock it.
 */
class ScopeUnlockOnly : public Cancelable, private Noncopyable
{
public:
	explicit ScopeUnlockOnly(Mutex& m) : Cancelable(), m(m) { }
	~ScopeUnlockOnly() { if(!isCanceled()) m.unlock(); }
	Mutex& m;
};	// ScopeUnlockOnly

//! Lock recursive mutex in scope.
class ScopeRecursiveLock : public Cancelable, private Noncopyable
{
public:
	explicit ScopeRecursiveLock(RecursiveMutex& m) : Cancelable(), m(m) { m.lock(); }
	~ScopeRecursiveLock() { if(!isCanceled()) m.unlock(); }
	RecursiveMutex& m;
};	// ScopeRecursiveLock

//! Unlocking recursive mutex in scope.
class ScopeRecursiveUnlockOnly : public Cancelable, private Noncopyable
{
public:
	explicit ScopeRecursiveUnlockOnly(RecursiveMutex& m) : Cancelable(), m(m) { }
	~ScopeRecursiveUnlockOnly() { if(!isCanceled()) m.unlock(); }
	RecursiveMutex& m;
};	// ScopeRecursiveUnlockOnly

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_MUTEX__
