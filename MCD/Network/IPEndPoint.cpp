#include "Pch.h"
#include "IPEndPoint.h"
#include "Platform.h"

namespace MCD {

IPEndPoint::IPEndPoint(const IPAddress& address, uint16_t port)
{
}

bool IPEndPoint::parse(sal_in_z const char* addressAndPort)
{
	return true;
}

const IPAddress& IPEndPoint::address() const {
	return mAddress;
}

void IPEndPoint::setAddress(const IPAddress& address) {
	mAddress = address;
}

uint16_t IPEndPoint::port() const {
	return uint16_t( ntohs(*(unsigned short*)(mAddress.nativeAddr().sa_data)) );
}

void IPEndPoint::setPort(uint16_t port) {
	*(unsigned short*)(mAddress.nativeAddr().sa_data) = htons((unsigned short)port);
}

bool IPEndPoint::operator==(const IPEndPoint& rhs) const {
	return mAddress == rhs.mAddress && port() == rhs.port();
}

bool IPEndPoint::operator<(const IPEndPoint& rhs) const
{
	if(mAddress < rhs.mAddress)
		return true;
	if(mAddress == rhs.mAddress)
		return port() < rhs.port();
	return false;
}

}	// namespace MCD
