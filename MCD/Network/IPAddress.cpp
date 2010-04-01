#include "Pch.h"
#include "IPAddress.h"
#include "Platform.h"
#include "../Core/System/StaticAssert.h"
#include <sstream>

namespace MCD {

namespace {

struct BigInt
{
	union {
		uint64_t m64;
		unsigned int mInt[2];
		unsigned short mShort[4];
		unsigned char mChar[8];
	};
};	// BigInt

}	// namespace

IPAddress::IPAddress()
{
	*this = getLoopBack();
}

IPAddress::IPAddress(uint64_t ip)
{
	// Clear the sockaddr
	::memset(mSockAddr, 0, sizeof(sockaddr));

	// TODO: Initialize mAddress->sa_family with AF_INET6 when we support IPv6
	nativeAddr().sa_family = AF_INET;
	BigInt& bi = reinterpret_cast<BigInt&>(ip);

	// Correct the byte order first
	bi.mShort[0] = htons(bi.mShort[0]);
	bi.mShort[1] = htons(bi.mShort[1]);

	::memcpy(&nativeAddr().sa_data[2], &bi, 4);
}

IPAddress::IPAddress(const sockaddr& ip)
{
	MCD_ASSERT(AF_INET == ip.sa_family);
	MCD_STATIC_ASSERT(sizeof(sockaddr) == sizeof(mSockAddr));
	nativeAddr() = ip;
}

bool IPAddress::parse(sal_in_z_opt const char* ipOrHostName)
{
	if(ipOrHostName == nullptr || ipOrHostName[0] == 0)
		return false;

	struct addrinfo hints;
	struct addrinfo* res = nullptr;

	::memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;

	int myerrno = ::getaddrinfo(ipOrHostName, nullptr, &hints, &res);
	if (myerrno != 0) {
		errno = myerrno;
		return false;
	}

	::memcpy(&nativeAddr().sa_data[2], &res->ai_addr->sa_data[2], 4);
	freeaddrinfo(res);

	return true;
}

IPAddress IPAddress::getLoopBack() {
	// 98048 will be encoded into 127.0.0.1
	return IPAddress(98048);
}

IPAddress IPAddress::getIPv6LoopBack() {
	MCD_ASSERT(false);
	return IPAddress(0);
}

IPAddress IPAddress::getAny() {
	return IPAddress(0);
}

IPAddress IPAddress::getIPv6Any()
{
	MCD_ASSERT(false);
	return IPAddress(0);
}

uint64_t IPAddress::getInt() const
{
	uint64_t ip = 0;
	BigInt& bi = reinterpret_cast<BigInt&>(ip);
	::memcpy(&bi, &nativeAddr().sa_data[2], 4);

	// Correct the byte order
	bi.mShort[0] = ntohs(bi.mShort[0]);
	bi.mShort[1] = ntohs(bi.mShort[1]);

	return ip;
}

std::string IPAddress::getString() const {
	std::ostringstream os;

	const sockaddr& addr = nativeAddr();
	// Note that the cast from uint8_t to size_t is necessary
	os	<< size_t(uint8_t(addr.sa_data[2])) << '.'
		<< size_t(uint8_t(addr.sa_data[3])) << '.'
		<< size_t(uint8_t(addr.sa_data[4])) << '.'
		<< size_t(uint8_t(addr.sa_data[5]));

	return os.str();
}

sockaddr& IPAddress::nativeAddr() const
{
	return const_cast<sockaddr&>(
		reinterpret_cast<const sockaddr&>(mSockAddr)
	);
}

bool IPAddress::operator==(const IPAddress& rhs) const {
	return getInt() == rhs.getInt();;
}

bool IPAddress::operator!=(const IPAddress& rhs) const {
	return !(*this == rhs);
}

bool IPAddress::operator<(const IPAddress& rhs) const {
	return getInt() < rhs.getInt();;
}

}	// namespace MCD
