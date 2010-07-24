#ifndef __MCD_NETWORK_PLATFORM__
#define __MCD_NETWORK_PLATFORM__

#include "ShareLib.h"
#include "../Core/System/Platform.h"

#if defined(MCD_WIN32)				// Native Windows
#	undef FD_SETSIZE
#	define FD_SETSIZE 10240
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	pragma comment(lib, "Ws2_32.lib")
#elif defined(_XENON)				// XBox360
#	include <winsockx.h>
#	ifndef NDEBUG
#		pragma comment(lib, "xnetd")
#		pragma comment(lib, "xbdm")
#		pragma comment(lib, "xboxkrnl")
#	else
#		pragma comment(lib, "xnet")
#	endif
#else								// Other POSIX OS
#	include <arpa/inet.h>
#	include <errno.h>
#	include <inttypes.h>
#	include <netdb.h>
#	include <netinet/in.h>
#	include <signal.h>
#	include <sys/socket.h>
#	include <sys/types.h>
#	include <unistd.h>
#endif

#endif	// __MCD_NETWORK_PLATFORM__
