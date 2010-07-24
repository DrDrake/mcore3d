#ifndef __MCD_NETWORK_IPADDRESS__
#define __MCD_NETWORK_IPADDRESS__

#include "ShareLib.h"
#include "../Core/System/Platform.h"
#include <string>

// Forward declaration of platform dependent type
struct sockaddr;

namespace MCD {

/*!	Represent an IP address
 */
class MCD_NETWORK_API IPAddress
{
public:
	//!	Default constructor gives loopback ip
	IPAddress();

	explicit IPAddress(const sockaddr& ip);

	explicit IPAddress(uint64_t ip);

// Operations
	//!	Null or empty string will result a loop back ip
	sal_checkreturn bool parse(sal_in_z_opt const char* ipOrHostName);

// Attributes
	//! Provides the IP loop back address.
	static IPAddress getLoopBack();
	static IPAddress getIPv6LoopBack();

	//! Provides an IP address that indicates that the server must listen for client activity on all network interfaces.
	static IPAddress getAny();
	static IPAddress getIPv6Any();

	uint64_t getInt() const;

	std::string getString() const;

	sockaddr& nativeAddr() const;	//! Function that cast mSockAddr into sockaddr

	bool operator==(const IPAddress& rhs) const;
	bool operator!=(const IPAddress& rhs) const;
	bool operator<(const IPAddress& rhs) const;

protected:
	char mSockAddr[16];
};	// SemanticMap

}	// namespace MCD

#endif	// __MCD_NETWORK_IPADDRESS__
