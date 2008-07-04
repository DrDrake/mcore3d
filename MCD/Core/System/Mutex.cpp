#include "Pch.h"
#include "Mutex.h"
#include "PlatformInclude.h"
#include "StaticAssert.h"

namespace MCD {

#ifdef MCD_WIN32

Mutex::Mutex()
{
	// If you see this static assert, please check the size of the CRITICAL_SECTION
	MCD_STATIC_ASSERT(sizeof(mMutex) == sizeof(CRITICAL_SECTION));
#ifndef NDEBUG
	_locked = false;
#endif
	::InitializeCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

Mutex::~Mutex()
{
#ifndef NDEBUG
	MCD_ASSUME(!_locked && "Delete before unlock");
#endif
	::DeleteCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void Mutex::lock()
{
	::EnterCriticalSection((LPCRITICAL_SECTION)&mMutex);
#ifndef NDEBUG
	MCD_ASSUME(!_locked && "Double lock");
	_locked = true;
#endif
}

void Mutex::unlock()
{
#ifndef NDEBUG
	MCD_ASSUME(_locked && "Unlock when not locked");
	_locked = false;
#endif
	::LeaveCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

//! Return true if already locked, otherwise false
bool Mutex::tryLock()
{
	if(::TryEnterCriticalSection((LPCRITICAL_SECTION)&mMutex) > 0) {
#ifndef NDEBUG
		MCD_ASSUME(!_locked && "Double lock");
		_locked = true;
#endif
		return true;
	} else
		return false;
}

RecursiveMutex::RecursiveMutex()
{
	::InitializeCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

RecursiveMutex::~RecursiveMutex()
{
	::DeleteCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void RecursiveMutex::lock()
{
	::EnterCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void RecursiveMutex::unlock()
{
	::LeaveCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

bool RecursiveMutex::tryLock()
{
	return ::TryEnterCriticalSection((LPCRITICAL_SECTION)&mMutex) > 0;
}

#else

Mutex::Mutex()
{
#ifndef NDEBUG
	_locked = false;
#endif
	::pthread_mutex_init(&mMutex, nullptr);
}

Mutex::~Mutex()
{
#ifndef NDEBUG
	MCD_ASSUME(!_locked);	// Delete before unlock
#endif
	::pthread_mutex_destroy(&mMutex);
}

void Mutex::lock()
{
	MCD_VERIFY(::pthread_mutex_lock(&mMutex) == 0);
#ifndef NDEBUG
	MCD_ASSERT(!_locked);	// Double lock
	_locked = true;
#endif
}

void Mutex::unlock()
{
#ifndef NDEBUG
	MCD_ASSUME(_locked);	// Unlock when not locked
	_locked = false;
#endif
	MCD_VERIFY(::pthread_mutex_unlock(&mMutex) == 0);
}

bool Mutex::tryLock()
{
	if(!::pthread_mutex_trylock(&mMutex)) {
#ifndef NDEBUG
		MCD_ASSUME(!_locked); //double lock
		_locked = true;
#endif
		return true;
	} else {
		return false;
	}
}

RecursiveMutex::RecursiveMutex()
{
	pthread_mutexattr_t attr;
	MCD_VERIFY(::pthread_mutexattr_init(&attr) == 0);
	MCD_VERIFY(::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);
	MCD_VERIFY(::pthread_mutex_init(&mMutex, &attr) == 0);
	MCD_VERIFY(::pthread_mutexattr_destroy(&attr) == 0);
}

RecursiveMutex::~RecursiveMutex()
{
	MCD_VERIFY(::pthread_mutex_destroy(&mMutex) == 0);
}

void RecursiveMutex::lock()
{
	::pthread_mutex_lock(&mMutex);
}

void RecursiveMutex::unlock()
{
	MCD_VERIFY(::pthread_mutex_unlock(&mMutex) == 0);
}

bool RecursiveMutex::tryLock()
{
	return !::pthread_mutex_trylock(&mMutex);
}

#endif	// MCD_WIN32

}	// namespace MCD
