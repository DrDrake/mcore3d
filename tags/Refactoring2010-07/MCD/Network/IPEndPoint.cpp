#include "Pch.h"
#include "IPEndPoint.h"
#include "Platform.h"
#include "../Core/System/StrUtility.h"
#include <sstream>

namespace MCD {

IPEndPoint::IPEndPoint(const sockaddr& addr)
	: mAddress(addr)
{
}

IPEndPoint::IPEndPoint(const IPAddress& address, uint16_t port)
	: mAddress(address)
{
	setPort(port);
}

bool IPEndPoint::parse(sal_in_z const char* addressAndPort)
{
	std::string str(addressAndPort);

	// Get position of colon
	size_t pos = str.find(":");

	if (pos != std::string::npos) {
		std::string strIP = str.substr(0, pos);
		if(!mAddress.parse(strIP.c_str()))
			return false;

		// Check existence of port number after colon
		if (str.length() > pos+1) {
			int port;
			if(str2Int(str.substr(pos+1).c_str(), port))
				setPort(uint16_t(port));
			else
				return false;
		}
	} else {
		return mAddress.parse(addressAndPort);
	}

	return true;
}

IPAddress& IPEndPoint::address() {
	return mAddress;
}

const IPAddress& IPEndPoint::address() const {
	return mAddress;
}

void IPEndPoint::setAddress(const IPAddress& address) {
	uint16_t p = port();
	mAddress = address;
	setPort(p);
}

uint16_t IPEndPoint::port() const {
	return uint16_t( ntohs(*(unsigned short*)(mAddress.nativeAddr().sa_data)) );
}

void IPEndPoint::setPort(uint16_t port) {
	*(unsigned short*)(mAddress.nativeAddr().sa_data) = htons((unsigned short)port);
}

std::string IPEndPoint::getString() const {
	std::stringstream ss;
	ss << mAddress.getString() << ":" << port();
	return ss.str();
}

bool IPEndPoint::operator==(const IPEndPoint& rhs) const {
	return mAddress == rhs.mAddress && port() == rhs.port();
}

bool IPEndPoint::operator!=(const IPEndPoint& rhs) const {
	return !(*this == rhs);
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
