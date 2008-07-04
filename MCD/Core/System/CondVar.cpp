#include "Pch.h"
#include "CondVar.h"
#include "PlatformInclude.h"
#include "Timer.h"
#include "Utility.h"

namespace MCD {

#ifdef MCD_WIN32

CondVar::CondVar()
	: Mutex(), mWaitCount(0), mBroadcastCount(0)
{
	// Create auto-reset
	h[0] = ::CreateEvent(nullptr, false, false, nullptr);
	if(!h[0])
		throwSystemErrorMessage("Error creating condition variable.");
	// Create manual-reset
	h[1] = ::CreateEvent(nullptr, true, false, nullptr);
	if(!h[1])
		throwSystemErrorMessage("Error creating condition variable.");
}

CondVar::~CondVar()
{
	if(h[0]) { MCD_VERIFY(::CloseHandle(h[0]) != 0); h[0] = nullptr; }
	if(h[1]) { MCD_VERIFY(::CloseHandle(h[1]) != 0); h[1] = nullptr; }
}

void CondVar::signalNoLock()
{
	MCD_ASSERT(_locked);
	if(::SetEvent(h[0]) == 0)
		throwSystemErrorMessage("CondVar failed to signal.");
}

void CondVar::broadcastNoLock()
{
	MCD_ASSERT(_locked);
	if(::SetEvent(h[1]) == 0)
		throwSystemErrorMessage("CondVar failed to broadcast.");
	mBroadcastCount = mWaitCount;
}

void CondVar::waitNoLock()
{
	_waitNoLock(INFINITE);
}

bool CondVar::waitNoLock(const TimeInterval& timeOut)
{
	return _waitNoLock(uint32_t(timeOut.asSecond() * 1000));
}

#pragma warning(push)
#pragma warning(disable: 4702)
bool CondVar::_waitNoLock(uint32_t milliseconds)
{
	MCD_ASSERT(_locked);
	++mWaitCount;

wait:
	this->unlock();
	DWORD ret = ::WaitForMultipleObjects(2, h, false, milliseconds);
	this->lock();
	--mWaitCount;

	switch(ret) {
	case WAIT_OBJECT_0:	return true;
	case WAIT_TIMEOUT:	return false;
	case WAIT_OBJECT_0+1:
		--mBroadcastCount;
		if(mBroadcastCount <= 0) {
			if(::ResetEvent(h[1]) == 0)
				throwSystemErrorMessage("CondVar failed to wait.");
			if(mBroadcastCount < 0)
				goto wait;
		}
		return true;
	case WAIT_ABANDONED:
	case WAIT_ABANDONED+1:
		throwSystemErrorMessage("CondVar abandon wait.");
		return false;
	default:
		MCD_ASSUME(false);
		return false;
	}
}
#pragma warning(pop)

#else

CondVar::CondVar() : Mutex()
{
	if(::pthread_cond_init(&c, nullptr) != 0)
		throwSystemErrorMessage("Error creating condition variable.");
}

CondVar::~CondVar()
{
	MCD_VERIFY(::pthread_cond_destroy(&c) == 0);
}

void CondVar::signalNoLock()
{
#ifndef NDEBUG
	MCD_ASSERT(_locked);
#endif
	::pthread_cond_signal(&c);
}

void CondVar::broadcastNoLock()
{
#ifndef NDEBUG
	MCD_ASSERT(_locked);
#endif
	::pthread_cond_broadcast(&c);
}

void CondVar::waitNoLock()
{
#ifndef NDEBUG
	MCD_ASSERT(_locked);
	_locked = false;
#endif
	::pthread_cond_wait(&c, &mMutex);
#ifndef NDEBUG
	MCD_ASSERT(!_locked);
	_locked = true;
#endif
}

bool CondVar::waitNoLock(const TimeInterval& timeOut)
{
	return _waitNoLock(uint32_t(timeOut.asSecond() * 1000 * 1000));
}

bool CondVar::_waitNoLock(useconds_t microseconds)
{
#ifndef NDEBUG
	MCD_ASSERT(_locked);
	_locked = false;
#endif
	int ret;
	timespec t;
	timeval val;

	::gettimeofday(&val, nullptr);
	t.tv_sec =  val.tv_sec + time_t(microseconds / 1e6);
	t.tv_nsec = (microseconds % 1000000) * 1000;
	ret = ::pthread_cond_timedwait(&c, &mMutex, &t);
#ifndef NDEBUG
	// TODO: Consider error handling on ret != 0
	MCD_ASSERT(!_locked);
	_locked = true;
#endif
	switch(ret) {
	case 0:			return true;
	case ETIMEDOUT:	return false;
	case EINVAL:
		throw std::invalid_argument("Invalid param given to ::pthread_cond_timedwait");
		return false;
	default:
		MCD_ASSUME(false);
		return false;
	}
}

#endif	// MCD_WIN32

}	// namespace MCD
