#include "Pch.h"
#include "../../MCD/Network/BsdSocket.h"
#include "../../MCD/Core/System/Thread.h"

using namespace MCD;

namespace {


class BsdSocketTestFixture
{
protected:
	BsdSocketTestFixture()
		: mLocalEndPoint(IPAddress::getLoopBack(), 1234)
		, mAnyEndPoint(IPAddress::getAny(), 1234)
	{
		MCD_VERIFY(BsdSocket::initApplication() == 0);
	}

	~BsdSocketTestFixture()
	{
		MCD_VERIFY(BsdSocket::closeApplication() == 0);
	}

	Thread mThread;
	IPEndPoint mLocalEndPoint;
	IPEndPoint mAnyEndPoint;
};	// BsdSocketTestFixture

struct SimpleConnector : public MCD::Thread::IRunnable
{
	SimpleConnector(const IPEndPoint& ep) : endPoint(ep) {}

	sal_override void run(Thread& thread) throw() {
		BsdSocket s;
		MCD_VERIFY(s.create(BsdSocket::TCP) == 0);
		bool connected = false;
		while(thread.keepRun()) {
			connected = s.connect(endPoint) == 0;

			if(connected)
				break;
		}
	}

	IPEndPoint endPoint;
};	// SimpleConnector

}	// namespace

TEST_FIXTURE(BsdSocketTestFixture, BlockingAcceptAndConnect)
{
	BsdSocket s1;
	CHECK_EQUAL(0, s1.create(BsdSocket::TCP));
	CHECK_EQUAL(0, s1.bind(mAnyEndPoint));
	CHECK_EQUAL(0, s1.listen());

	SimpleConnector connector(mLocalEndPoint);
	Thread thread(connector, false);

	BsdSocket s2;
	CHECK(s1.accept(s2) == 0);

	thread.wait();
}

TEST_FIXTURE(BsdSocketTestFixture, NonBlockingAccept)
{
	BsdSocket s1;
	CHECK_EQUAL(0, s1.create(BsdSocket::TCP));
	CHECK_EQUAL(0, s1.bind(mAnyEndPoint));
	CHECK_EQUAL(0, s1.listen());

	BsdSocket s2;
	CHECK(s1.setBlocking(false) == 0);
	CHECK(BsdSocket::inProgress(s1.accept(s2)));
	CHECK(BsdSocket::inProgress(s1.lastError));

	SimpleConnector connector(mLocalEndPoint);
	Thread thread(connector, false);

	while(BsdSocket::inProgress(s1.accept(s2))) {}

	thread.wait();
}

class SocketPoller
{
public:
};
