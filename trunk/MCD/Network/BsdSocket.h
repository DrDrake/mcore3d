#ifndef __MCD_NETWORK_BSDSOCKET__
#define __MCD_NETWORK_BSDSOCKET__

#include "IPEndPoint.h"
#include "../Core/System/NonCopyable.h"

// Forward declaration of platform dependent type
typedef uint socket_t;

namespace MCD {

//! Cross-platform BSD socket class
class MCD_NETWORK_API BsdSocket : Noncopyable
{
public:
	//!	Zero for no error
	typedef int ErrorCode;

	enum SocketType {
		Undefined,
		TCP,
		TCP6,
		UDP,
		UDP6
	};	// SocketType

	//! Call this before you use any function of BsdSocket
	static sal_checkreturn ErrorCode initApplication();

	//! Call this before your main exist
	static sal_checkreturn ErrorCode closeApplication();

	BsdSocket();

	~BsdSocket();

// Operations
	sal_checkreturn ErrorCode create(SocketType type);

	sal_checkreturn ErrorCode setBlocking(bool block);

	sal_checkreturn ErrorCode bind(const IPEndPoint& endPoint);

	/*!	Places the socket in a listening state
		\param backlog Specifies the number of incoming connections that can be queued for acceptance
	 */
	sal_checkreturn ErrorCode listen(size_t backlog=5);

	/*!	Creates a new Socket for a newly created connection.
		Accept extracts the first connection on the queue of pending connections on this socket.
		It then returns a the newly connected socket that will handle the actual connection.
	 */
	sal_checkreturn ErrorCode accept(BsdSocket& socket) const;

	//! Establishes a connection to a remote host.
	sal_checkreturn ErrorCode connect(const IPEndPoint& endPoint);

	//!	Returns -1 for any error
	sal_checkreturn ssize_t send(const void* data, size_t len, int flags=0);

	//!	Returns -1 for any error
	sal_checkreturn ssize_t receive(void* buf, size_t len, int flags=0);

	//!	Returns -1 for any error
	sal_checkreturn ssize_t sendTo(const void* data, size_t len, const IPEndPoint& destEndPoint, int flags=0);

	//!	Returns -1 for any error
	sal_checkreturn ssize_t receiveFrom(void* buf, size_t len, IPEndPoint& srcEndPoint, int flags=0);

	/*	To assure that all data is sent and received on a connected socket before it is closed,
		an application should use ShutDownXXX() to close connection before calling close().
		Reference: See MSDN on ::shutdown
	 */

	/*!	Shutdown read
		Do nothing if fd() is invalid.
	 */
	sal_checkreturn ErrorCode shutDownRead();

	/*!	Shutdown write
		Do nothing if fd() is invalid.
	 */
	sal_checkreturn ErrorCode shutDownWrite();

	/*!	Shutdown read and write
		Do nothing if fd() is invalid.
	 */
	sal_checkreturn ErrorCode shutDownReadWrite();

	//! Close the socket
	void requestClose();

	//!	Do nothing if fd() is invalid.
	sal_checkreturn ErrorCode close();

// Attributes
	//! Whether the socket is bound to a specific local port.
	bool IsBound() const;

	/*!	Gets an IPEndPoint that contains the local IP address and port number to which your socket is bounded
		Throw if the socket is not bounded
	 */
	IPEndPoint boundEndPoint() const;

	/*!	If you are using a connection-oriented protocol, it gets an IPEndPoint that contains
		the remote IP address and port number to which your socket is connected to.
		If
		note/ GetRemoteEndPoint is implemented using ::getpeername
		Throw if the socket is not connected
	 */
	IPEndPoint remoteEndPoint() const;

	mutable ErrorCode lastError;

	/*! Check to see if there is system error.
		\return true if there is system error. Otherwise false.
	 */
	bool IsError();

	/*! Store the system error code to the parameter.
		\param errorCode the place the return error code will be stored in
		\return at this moment this method always returns 0.
	 */
	bool GetErrorCode(int& errorCode) const;

	//!
	bool GetSocketErrorCode(int& errorCode) const;

	//!	Check the error code whether it indicating a socket operations is in progress.
	static bool inProgress(int code);

	const socket_t& fd() const;
	void setFd(const socket_t& f);

protected:
	char mFd[4];	//!< File descriptor
	IPEndPoint mLocalEndPoint;
	mutable bool mIsConnected;
};	// BsdSocket

}	// namespace MCD

#endif	// __MCD_NETWORK_BSDSOCKET__
