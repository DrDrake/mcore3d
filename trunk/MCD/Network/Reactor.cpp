#include "Pch.h"
#include "Reactor.h"
#include "Platform.h"

namespace MCD {

void intrusivePtrRelease(Socket* s) {
	// NOTE: Gcc4.2 failed to compile "--(p->mRefCount)" correctly.
	s->mRefCount--;
	if(s->mRefCount == 0) {
//		s->destructionLock();	// NOTE: We preform the lock before deleting.
//		ScopeLock lock(s->mReactor.mutex);
		delete s;
	}
}

Socket::~Socket()
{
	MCD_ASSERT(mReactor.mutex.isLocked());
	MCD_ASSERT(mRefCount == 0 && "Don't destroy this directory, use shared pointer");
}

Socket::ErrorCode Socket::connect(const IPEndPoint& endPoint)
{
	return 0;
}

void Socket::retain() {
	intrusivePtrAddRef(this);
}

void Socket::release() {
	intrusivePtrRelease(this);
}

SocketPtr Reactor::create(BsdSocket::SocketType type)
{
	SocketPtr s = socketFactory(type);

	if(s->BsdSocket::create(type) != 0) return nullptr;
	if(s->BsdSocket::setBlocking(false) != 0) return nullptr;

	ScopeLock lock(mutex);
	mSockets.pushBack(*s);

	// Udp socket can perform IO immediatly
	if(type == BsdSocket::UDP)
		mIoSockets.pushBack(s->mActive);

	return s;
}

SocketPtr Reactor::socketFactory(BsdSocket::SocketType type)
{
	return new Socket(*this);
}

BsdSocket::ErrorCode Reactor::listen(const IPEndPoint& endPoint, SocketPtr& acceptor)
{
	acceptor = create(BsdSocket::TCP);
	if(!acceptor)
		return -1;

	if(acceptor->bind(endPoint) != 0) return acceptor->lastError;
	if(acceptor->listen() != 0) return acceptor->lastError;

	ScopeLock lock(mutex);
	mAcceptorSockets.pushBack(acceptor->mActive);
	return 0;
}

// Note that those redundant fd_set* is needed to prevent suprise made by macro argument
#define UPDATE_FDSETS(s, max, rdset, wtset, exset) {					\
	fd_set* _rdset = (rdset), * _wtset = (wtset), * _exset = (exset);	\
	if((int)(s) > (max)) max = (int)(s);								\
	if(_rdset) FD_SET((s), _rdset);										\
	if(_wtset) FD_SET((s), _wtset);										\
	if(_exset) FD_SET((s), _exset);										\
}

void Reactor::process()
{
	int maxFD = -1;
	fd_set readSet;
	fd_set writeSet;
	fd_set errorSet;
	timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);

	ScopeLock lock(mutex);

	// Check IO mSockets
	for(Socket::Active* a = mIoSockets.begin(); a != mIoSockets.end(); a = a->next()) {
		ScopeUnlock unlock(mutex);
		Socket& s = a->getOuter();
		UPDATE_FDSETS(s.fd(), maxFD, &readSet, &writeSet, &errorSet);
	}

	// Check acceptor mSockets
	for(Socket::Active* a = mAcceptorSockets.begin(); a != mAcceptorSockets.end(); a = a->next()) {
		ScopeUnlock unlock(mutex);
		Socket& s = a->getOuter();
		UPDATE_FDSETS(s.fd(), maxFD, &readSet, nullptr, &errorSet);
	}

	// Check connecting mSockets
	for(Socket::Active* a = mConnectingSockets.begin(); a != mConnectingSockets.end(); a = a->next()) {
		ScopeUnlock unlock(mutex);
		Socket& s = a->getOuter();
		UPDATE_FDSETS(s.fd(), maxFD, nullptr, &writeSet, &errorSet);
	}

	::select(maxFD+1, &readSet, &writeSet, &errorSet, &tv);

	// Check IO mSockets
	for(Socket::Active* a = mIoSockets.begin(); a != mIoSockets.end(); ) {
		ScopeUnlock unlock(mutex);
		SocketWeakPtr s = &a->getOuter();
		const socket_t& fd = s->fd();
		a = a->next();

		if(FD_ISSET(fd, &errorSet))
			onError(*s);

		if(FD_ISSET(fd, &readSet) && s)
			onReadReady(*s);

		if(FD_ISSET(fd, &writeSet) && s)
			onWriteReady(*s);
	}

	// Check acceptor mSockets
	for(Socket::Active* a = mAcceptorSockets.begin(); a != mAcceptorSockets.end(); ) {
		ScopeUnlock unlock(mutex);
		SocketWeakPtr s = &a->getOuter();
		const socket_t& fd = s->fd();
		a = a->next();

		if(FD_ISSET(fd, &errorSet))
			onError(*s);

		if(FD_ISSET(fd, &readSet) && s) {
			SocketPtr s2 = create(BsdSocket::TCP);
			if(s->accept(*s2) == 0) {
				onAccepted(*s, *s2);
				mIoSockets.pushBack(s2->mActive);
			}
			else {
				MCD_ASSERT(false && "The error should be already caught by the excep fd set");
			}
		}
	}

	// Check connecting mSockets
	for(Socket::Active* a = mConnectingSockets.begin(); a != mConnectingSockets.end(); ) {
		ScopeUnlock unlock(mutex);
		SocketWeakPtr s = &a->getOuter();
		const socket_t& fd = s->fd();
		a = a->next();

		if(FD_ISSET(fd, &errorSet))
			onError(*s);

		if(FD_ISSET(fd, &writeSet) && s)
			onConnected(*s);
	}
}

}	// namespace MCD
