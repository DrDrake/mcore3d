#ifndef __MCD_NETWORK_IPENDPOINT__
#define __MCD_NETWORK_IPENDPOINT__

#include "IPAddress.h"

namespace MCD {

/*!	Represent an IP address with a port
 */
class MCD_NETWORK_API IPEndPoint
{
public:
	explicit IPEndPoint(const sockaddr& addr);

	//!	Default constructor gives loopback ip
	IPEndPoint(const IPAddress& address, uint16_t port);

// Operations
	//!	String format: address:port
	sal_checkreturn bool parse(sal_in_z const char* addressAndPort);

// Attributes
	IPAddress& address();
	const IPAddress& address() const;
	void setAddress(const IPAddress& address);

	uint16_t port() const;
	void setPort(uint16_t port);

	std::string getString() const;

	bool operator==(const IPEndPoint& rhs) const;
	bool operator!=(const IPEndPoint& rhs) const;
	bool operator<(const IPEndPoint& rhs) const;

protected:
	IPAddress mAddress;
};	// IPEndPoint

}	// namespace MCD

#endif	// __MCD_NETWORK_IPENDPOINT__
