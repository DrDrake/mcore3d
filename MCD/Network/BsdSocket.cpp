#include "Pch.h"
#include "BsdSocket.h"
#include "Platform.h"
#include "../Core/System/StaticAssert.h"
#include <fcntl.h>

// A good guide on socket programming:
// http://beej.us/guide/bgnet/output/html/multipage/index.html

#ifndef SD_RECEIVE
#	define SD_RECEIVE	0x00
#	define SD_SEND		0x01
#	define SD_BOTH		0x02
#endif

namespace MCD {

#if defined(MCD_WIN)
//	typedef int			socklen_t;
//	typedef UINT_PTR	SOCKET;
//	typedef UINT_PTR	socket_t;
#endif

#ifdef OK
#	undef OK
#endif	// OK

// Unify the error code used in Linux and win32
#if defined(MCD_WIN)
#	define OK			S_OK
#	define EALREADY		WSAEALREADY		// Operation already in progress
#	define ECONNABORTED	WSAECONNABORTED	// Software caused connection abort
#	define ECONNRESET	WSAECONNRESET	// Connection reset by peer
#	define EHOSTDOWN	WSAEHOSTDOWN	// Host is down
#	define EHOSTUNREACH	WSAEHOSTUNREACH	// No route to host
#	define EINPROGRESS	WSAEINPROGRESS	// Operation now in progress
#	define ENETDOWN		WSAENETDOWN		// Network is down
#	define ENETRESET	WSAENETRESET	// Network dropped connection on reset
#	define ENOBUFS		WSAENOBUFS		// No buffer space available (recoverable)
#	define ENOTCONN		WSAENOTCONN		// Socket is not connected
#	define ENOTSOCK		WSAENOTSOCK		// Socket operation on nonsocket
#	define ETIMEDOUT	WSAETIMEDOUT	// Connection timed out
#	define EWOULDBLOCK	WSAEWOULDBLOCK	// Operation would block (recoverable)
#else
#	define OK			0
#	define SOCKET_ERROR -1
#	define INVALID_SOCKET -1
#endif

#if defined(MCD_APPLE)
#	define MSG_NOSIGNAL 0x2000	// http://lists.apple.com/archives/macnetworkprog/2002/Dec/msg00091.html
#endif

static int getLastError()
{
#if defined(MCD_WIN)
	return WSAGetLastError();
#else
//	if(errno != 0)
//		perror("Network error:");
	return errno;
#endif
}

static int toInt(size_t s) {
	return (s >= 0x7fffffff) ? 0x7fffffff : (int)s;
}

typedef BsdSocket::ErrorCode ErrorCode;

ErrorCode BsdSocket::initApplication()
{
#if defined(MCD_WIN)
	WSADATA	wsad;

	// Note that we cannot use GetLastError to determine the error code
	// as is normally done in Windows Sockets if WSAStartup fails.
	return ::WSAStartup(WINSOCK_VERSION, &wsad);
#else
	return OK;
#endif
}

ErrorCode BsdSocket::closeApplication()
{
#if defined(MCD_WIN)
	return ::WSACleanup();
#else
	return OK;
#endif
}

BsdSocket::BsdSocket()
	: lastError(0)
	, mLocalEndPoint(IPAddress::getAny(), 0)
{
	setFd(INVALID_SOCKET);
}

BsdSocket::~BsdSocket()
{
	MCD_STATIC_ASSERT(sizeof(socket_t) == sizeof(mFd));
	MCD_VERIFY(close() == OK);
}

ErrorCode BsdSocket::create(SocketType type)
{
	// If this socket is not closed yet
	if(fd() != INVALID_SOCKET)
		return lastError = -1;

	switch (type) {
	case TCP:	setFd(::socket(AF_INET, SOCK_STREAM, 0));	break;
	case TCP6:	setFd(::socket(AF_INET6, SOCK_STREAM, 0));	break;
	case UDP:	setFd(::socket(AF_INET, SOCK_DGRAM, 0));	break;
	case UDP6:	setFd(::socket(AF_INET6, SOCK_DGRAM, 0));	break;
	default: return lastError = getLastError();
	}

#ifndef MCD_WIN
	{	// Disable SIGPIPE: http://unix.derkeiler.com/Mailing-Lists/FreeBSD/net/2007-03/msg00007.html
		// More reference: http://beej.us/guide/bgnet/output/html/multipage/sendman.html
		// http://discuss.joelonsoftware.com/default.asp?design.4.575720.7
		int b = 1;
		MCD_VERIFY(setsockopt(fd(), SOL_SOCKET, SO_NOSIGPIPE, &b, sizeof(b)) == 0);
	}
#endif

	return lastError = OK;
}

ErrorCode BsdSocket::setBlocking(bool block)
{
#if defined(MCD_WIN)
	unsigned long a = block ? 0 : 1;
	return lastError =
		ioctlsocket(fd(), FIONBIO, &a) == OK ?
		OK : getLastError();
#else
	return lastError =
		fcntl(fd(), F_SETFL, fcntl(fd(), F_GETFL) | O_NONBLOCK) != -1 ?
		OK : getLastError();
#endif
}

ErrorCode BsdSocket::bind(const IPEndPoint& endPoint)
{
	sockaddr addr = endPoint.address().nativeAddr();
	return lastError =
		::bind(fd(), &addr, sizeof(addr)) == OK ?
		OK : getLastError();
}

ErrorCode BsdSocket::listen(size_t backlog)
{
	return lastError =
		::listen(fd(), int(backlog)) == OK ?
		OK : getLastError();
}

ErrorCode BsdSocket::accept(BsdSocket& socket) const
{
	socket_t s;
	sockaddr addr;
	socklen_t len = sizeof(addr);

	s = ::accept(fd(), &addr, &len);
	if(s == INVALID_SOCKET)
		return lastError = getLastError();

	socket.setFd(s);
	return lastError = OK;
}

ErrorCode BsdSocket::connect(const IPEndPoint& endPoint)
{
	sockaddr addr = endPoint.address().nativeAddr();
	return lastError =
		::connect(fd(), &addr, sizeof(addr)) == OK ?
		OK : getLastError();
}

ssize_t BsdSocket::send(const void* data, size_t len, int flags)
{
	ssize_t ret = ::send(fd(), (const char*)data, toInt(len), flags);
	lastError = ret < 0 ? getLastError() : OK;
	return ret;
}

ssize_t BsdSocket::receive(void* buf, size_t len, int flags)
{
	ssize_t ret = ::recv(fd(), (char*)buf, toInt(len), flags);
	lastError = ret < 0 ? getLastError() : OK;
	return ret;
}

ssize_t BsdSocket::sendTo(const void* data, size_t len, const IPEndPoint& destEndPoint, int flags)
{
	sockaddr addr = destEndPoint.address().nativeAddr();
	ssize_t ret = ::sendto(fd(), (const char*)data, toInt(len), flags, &addr, sizeof(addr));
	lastError = ret < 0 ? getLastError() : OK;
	return ret;
}

ssize_t BsdSocket::receiveFrom(void* buf, size_t len, IPEndPoint& srcEndPoint, int flags)
{
	sockaddr& addr = srcEndPoint.address().nativeAddr();
	socklen_t bufSize = sizeof(addr);
	ssize_t ret = ::recvfrom(fd(), (char*)buf, toInt(len), flags, &addr, &bufSize);
	MCD_ASSERT(bufSize == sizeof(addr));
	lastError = ret < 0 ? getLastError() : OK;
	return ret;
}

ErrorCode BsdSocket::shutDownRead()
{
	if(fd() == INVALID_SOCKET || ::shutdown(fd(), SD_RECEIVE) == OK)
		return lastError = OK;

#ifdef MCD_APPLE
	return lastError = OK;
#else
	return lastError = getLastError();
#endif
}

ErrorCode BsdSocket::shutDownWrite()
{
	if(fd() == INVALID_SOCKET || ::shutdown(fd(), SD_SEND) == OK)
		return lastError = OK;

#ifdef MCD_APPLE
	return lastError = OK;
#else
	return lastError = getLastError();
#endif
}

ErrorCode BsdSocket::shutDownReadWrite()
{
	if(fd() == INVALID_SOCKET || ::shutdown(fd(), SD_BOTH) == OK)
		return lastError = OK;

#ifdef MCD_APPLE
	return lastError = OK;
#else
	return lastError = getLastError();
#endif
}

ErrorCode BsdSocket::close()
{
	if(fd() == INVALID_SOCKET)
		return lastError = OK;

#if defined(MCD_WIN)
	if(::closesocket(fd()) == OK)
		return lastError = OK;
#else
	if(::close(fd()) == OK)
		return lastError = OK;
#endif

#ifdef MCD_APPLE
	return lastError = OK;
#else
	return lastError = getLastError();
#endif
}

IPEndPoint BsdSocket::remoteEndPoint() const
{
	sockaddr addr;
	::memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	if(::getpeername(fd(), &addr, &len) == OK)
		return IPEndPoint(addr);
	return IPEndPoint(addr);
}

bool BsdSocket::inProgress(int code)
{
	return code == EINPROGRESS || code == EWOULDBLOCK;
}

const socket_t& BsdSocket::fd() const {
	return *reinterpret_cast<const socket_t*>(mFd);
}

void BsdSocket::setFd(const socket_t& f) {
	*reinterpret_cast<socket_t*>(mFd) = f;
}

}	// namespace MCD
