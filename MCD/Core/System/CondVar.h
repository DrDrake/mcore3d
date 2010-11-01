#ifndef __MCD_CORE_SYSTEM_CONDVAR__
#define __MCD_CORE_SYSTEM_CONDVAR__

#include "Mutex.h"

namespace MCD {

class TimeInterval;

/*!	Condition variable.
	Conditions (also known as condition queues or condition variables) provide
	a means for one thread to suspend execution (to "wait") until notified by
	another thread that some state condition may now be true.

	Because access to this shared state information occurs in different threads,
	it must be protected, so a lock of some form is associated with the condition.
	The key property that waiting for a condition provides is that it atomically
	releases the associated lock and suspends the current thread.

	A CondVar instance is intrinsically bound to a Mutex.
 */
class MCD_CORE_API CondVar : public Mutex
{
public:
	CondVar();
	~CondVar();

	/*! Causes the current thread to wait until it is signaled.
		The lock associated with this Condition is atomically released and the
		current thread becomes disabled for thread scheduling purposes and lies
		dormant until one of the following happens:
			-Some other thread invokes the signal() method for this CondVar and
			the current thread happens to be chosen as the thread to be awakened; or
			-Some other thread invokes the SignalAll() method for this CondVar.

		\note The implementation (especially on windows) is subject to "Spurious wakeup",
			see http://en.wikipedia.org/wiki/Spurious_wakeup for details.
	 */
	void wait() {
		ScopeLock lock(*this);
		waitNoLock();
	}
	void waitNoLock();

	/*!	Wait with timeout.
	 	\return True if signaled otherwise false when timeout.
	 */
	sal_checkreturn bool wait(const TimeInterval& timeout) {
		ScopeLock lock(*this);
		return waitNoLock(timeout);
	}
	sal_checkreturn bool waitNoLock(const TimeInterval& timeout);

	/*!	Wakes up one waiting thread.
		If any threads are waiting on this condition then one is selected for
		waking up. That thread must then re-acquire the lock before returning from await.
	 */
	void signal() {
		ScopeLock lock(*this);
		signalNoLock();
	}
	void signalNoLock();

	/*!	Wakes up all waiting threads.
		If any threads are waiting on this condition then they are all woken up.
		Each thread must re-acquire the lock before it can return from await.
	 */
	void broadcast() {
		ScopeLock lock(*this);
		broadcastNoLock();
	}
	void broadcastNoLock();

private:
#ifdef MCD_WIN
	bool _waitNoLock(uint32_t milliseconds);
	void* h[2];	// h[0]:signal, h[1]:broadcast
	int mWaitCount;
	int mBroadcastCount;
#else
	bool _waitNoLock(useconds_t microseconds);
	pthread_cond_t c;
#endif
};	// CondVar

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_CONDVAR__
