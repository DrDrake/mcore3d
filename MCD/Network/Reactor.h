#ifndef __MCD_NETWORK_REACTOR__
#define __MCD_NETWORK_REACTOR__

#include "BsdSocket.h"
#include "../Core/System/Atomic.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/Macros.h"
#include "../Core/System/WeakPtr.h"

namespace MCD {

class Reactor;

/*!	The life time of the Socket is handled by user and Reactor co-operatively.
 */
// TODO: Put this class into another namespce or inner class or rename it.
class MCD_NETWORK_API Socket : public BsdSocket, public IntrusiveSharedWeakPtrTarget<AtomicInteger>, protected LinkListBase::Node<Socket>
{
	friend class Reactor;

	//!	Represent the current active state
	struct Active : public LinkListBase::Node<Active> {
		MCD_DECLAR_GET_OUTER_OBJ(Socket, mActive);
		void destroyThis() {
			delete getOuterSafe();
		}
	} mActive;

	friend class LinkList<Socket>;
	friend class LinkList<Socket::Active>;

	//!	Traps all destruction to perform necessary locking
	friend MCD_NETWORK_API void intrusivePtrRelease(Socket* s);

protected:
	//!	Hide some functions that should be only called by Reactor
	Socket(Reactor& reactor) : mReactor(reactor) {}
	~Socket();
	ErrorCode create(SocketType type);
	ErrorCode setBlocking(bool block);

	Reactor& mReactor;

public:
	ErrorCode connect(const IPEndPoint& endPoint);

	void retain();
	void release();
};	// Socket

typedef IntrusivePtr<Socket> SocketPtr;
typedef IntrusiveWeakPtr<Socket> SocketWeakPtr;

class MCD_NETWORK_API Reactor
{
public:
	~Reactor() {}

// Operations
	virtual SocketPtr create(BsdSocket::SocketType type);

	BsdSocket::ErrorCode listen(const IPEndPoint& endPoint, SocketPtr& acceptor);

	void process();

	virtual void onConnected(Socket& s) {}

	virtual void onAccepted(Socket& acceptor, Socket& remote) {}

	virtual void onReadReady(Socket& s) {}

	virtual void onWriteReady(Socket& s) {}

	virtual void onError(Socket& s) {}

// Attributes
	LinkList<Socket> sockets() const;					//!< Contains all sockets that are managed by the Reactor.
	LinkList<Socket::Active> ioSockets() const;			//!< Sockets that are able to perform IO.
	LinkList<Socket::Active> acceptorSockets() const;
	LinkList<Socket::Active> connectingSockets() const;

	mutable Mutex mutex;

protected:
	virtual SocketPtr socketFactory(BsdSocket::SocketType type);

	LinkList<Socket> mSockets;
	LinkList<Socket::Active> mIoSockets;
	LinkList<Socket::Active> mAcceptorSockets;
	LinkList<Socket::Active> mConnectingSockets;
};	// Reactor

}	// namespace MCD

#endif	// __MCD_NETWORK_REACTOR__
