#include "Pch.h"
#include "../../MCD/Network/BsdSocket.h"
#include "../../MCD/Core/System/Thread.h"

using namespace MCD;

namespace {

class IPEndPointTestFixture
{
protected:
	IPEndPointTestFixture()
	{
		MCD_VERIFY(BsdSocket::initApplication() == 0);
	}

	~IPEndPointTestFixture()
	{
		MCD_VERIFY(BsdSocket::closeApplication() == 0);
	}
};	// IPEndPointTestFixture

}	// namespace

TEST_FIXTURE(IPEndPointTestFixture, Construct)
{
	// Construct from a TIPAddress (IP) and an integer (port)
	IPEndPoint ep1(IPAddress(0), 0u);
	IPEndPoint ep2(IPAddress::getLoopBack(), 0xffffu);

	// Construct from a string
	CHECK(!ep1.parse(""));
	CHECK(ep1.parse("0.0.0.0:0"));
	CHECK(ep1.parse("127.0.0.1:80"));
	CHECK(ep1.parse("localhost:80"));
//	CHECK(ep1.parse("yahoo.com:45678"));

	// Invalid address
	CHECK(!ep1.parse("253.254.255.256:80"));

	// Invalid port
	CHECK(!ep1.parse("localhost:port1234"));
}

TEST_FIXTURE(IPEndPointTestFixture, Getter)
{
	IPEndPoint ep1(IPAddress(0), 0u);
	CHECK(ep1.parse("127.0.0.1:80"));
	CHECK(ep1.address() == IPAddress::getLoopBack());
	CHECK_EQUAL(80u, ep1.port());

	//TIPEndPoint ep2("127.001:65535");
	//CHECK(ep2.GetAddress() == TIPAddress("127.0.0.1"));
	//CHECK(ep2.GetPort() == 65535);

	CHECK(ep1.parse("google.com:80"));
	CHECK_EQUAL("google.com:80", ep1.getString());
	CHECK_EQUAL("google.com", ep1.address().getString());
	CHECK_EQUAL(80u, ep1.port());
}

TEST_FIXTURE(IPEndPointTestFixture, Comparison)
{
	IPEndPoint ep1(IPAddress(0), 0u);
	CHECK(ep1.parse("127.0.0.1:80"));

	IPEndPoint ep2(IPAddress(0), 0u);
	CHECK(ep2.parse("localhost:80"));

	IPEndPoint ep3(IPAddress(0), 0u);

	CHECK(ep1 == ep2);
	CHECK(ep2 != ep3);
}
