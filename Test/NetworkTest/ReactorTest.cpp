#include "Pch.h"
#include "../../MCD/Network/BsdSocket.h"
#include "../../MCD/Network/Reactor.h"
#include "../../MCD/Core/System/Thread.h"

using namespace MCD;

namespace {

class ReactorTestFixture
{
protected:
	ReactorTestFixture()
		: mLocalEndPoint(IPAddress::getLoopBack(), 1234)
		, mAnyEndPoint(IPAddress::getAny(), 1234)
	{
		MCD_VERIFY(BsdSocket::initApplication() == 0);
	}

	~ReactorTestFixture()
	{
		MCD_VERIFY(BsdSocket::closeApplication() == 0);
	}

	// Quickly create a listening socket
	BsdSocket::ErrorCode listenOn(BsdSocket& s)
	{
		if(s.create(BsdSocket::TCP) != 0) return s.lastError;
		if(s.bind(mAnyEndPoint) != 0) return s.lastError;
		return s.listen();
	}

	Thread mThread;
	IPEndPoint mLocalEndPoint;
	IPEndPoint mAnyEndPoint;
};	// ReactorTestFixture

}

TEST_FIXTURE(ReactorTestFixture, Accept)
{
	struct MyReactor : public Reactor {
		MyReactor() : passed(false), finished(false) {}

		sal_override void onAccepted(Socket& acceptor, Socket& remote) {
			remote.retain();	// The reactor take no ownership of the accepted socket.
			passed = true;
		}

		sal_override void onReadReady(Socket& s) {
			char buf[64];
			if(s.receive(buf, sizeof(buf)) == 0) {
				s.release();
				finished = true;
			}
		}

		sal_override void onError(Socket& s) {
		}

		bool passed;
		bool finished;
	};	// MyReactor

	MyReactor reactor;

	SocketPtr acceptor;
	CHECK_EQUAL(0, reactor.listen(mAnyEndPoint, acceptor));

	BsdSocket s;
	CHECK(s.create(BsdSocket::TCP));
	CHECK(s.setBlocking(false));
	CHECK(s.connect(mLocalEndPoint));

	while(!reactor.passed) {
		reactor.process();
	}

	CHECK(s.shutDownReadWrite());

	while(!reactor.finished) {
		reactor.process();
	}
}
