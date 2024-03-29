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

	// Quickly create a listening socket
	BsdSocket::ErrorCode listenOn(BsdSocket& s)
	{
		if(s.create(BsdSocket::TCP) != 0) return s.lastError;
		if(s.bind(mAnyEndPoint) != 0) return s.lastError;
		return s.listen();
	}

	//!	Using non-blocking mode to setup a connection quickly
	BsdSocket::ErrorCode setupConnectedTcpSockets(BsdSocket& listener, BsdSocket& acceptor, BsdSocket& connector)
	{
		if(listenOn(listener) != 0) return listener.lastError;
		if(listener.setBlocking(false) != 0) return listener.lastError;

		if(!BsdSocket::inProgress(listener.accept(acceptor))) return listener.lastError;

		if(connector.create(BsdSocket::TCP) != 0) return listener.lastError;
		while(BsdSocket::inProgress(connector.connect(mLocalEndPoint))) {}
		if(connector.setBlocking(false) != 0) return listener.lastError;

		while(BsdSocket::inProgress(listener.accept(acceptor))) {}
		return acceptor.setBlocking(false);
	}

	Thread mThread;
	IPEndPoint mLocalEndPoint;
	IPEndPoint mAnyEndPoint;
};	// BsdSocketTestFixture

struct SimpleConnector : public MCD::Thread::IRunnable
{
	SimpleConnector(const IPEndPoint& ep) : endPoint(ep) {}

	sal_override void run(Thread& thread) {
		BsdSocket s;
		MCD_VERIFY(0 == s.create(BsdSocket::TCP));
		bool connected = false;
		while(thread.keepRun()) {
			connected = s.connect(endPoint) == 0;

			if(connected) {
				const char msg[] = "Hello world!";
				MCD_VERIFY(sizeof(msg) == s.send(msg, sizeof(msg)));
				break;
			}
		}
	}

	IPEndPoint endPoint;
};	// SimpleConnector

}	// namespace

TEST_FIXTURE(BsdSocketTestFixture, BlockingAcceptAndConnect)
{
	BsdSocket s1;
	CHECK_EQUAL(0, listenOn(s1));

	SimpleConnector connector(mLocalEndPoint);
	Thread thread(connector, false);

	BsdSocket s2;
	CHECK_EQUAL(0, s1.accept(s2));

	thread.wait();
}

TEST_FIXTURE(BsdSocketTestFixture, NonBlockingAccept)
{
	BsdSocket s1;
	CHECK_EQUAL(0, listenOn(s1));

	BsdSocket s2;
	CHECK_EQUAL(0, s1.setBlocking(false));
	CHECK(BsdSocket::inProgress(s1.accept(s2)));
	CHECK(BsdSocket::inProgress(s1.lastError));

	SimpleConnector connector(mLocalEndPoint);
	Thread thread(connector, false);

	while(BsdSocket::inProgress(s1.accept(s2))) {}

	thread.wait();
}

TEST_FIXTURE(BsdSocketTestFixture, TCPBlockingSendBlockingRecv)
{
	BsdSocket s1;
	CHECK_EQUAL(0, listenOn(s1));

	SimpleConnector connector(mLocalEndPoint);
	Thread thread(connector, false);

	BsdSocket s2;
	CHECK_EQUAL(0, s1.accept(s2));

	char buf[64];
	ssize_t totalRead = 0;
	ssize_t receivedSize = 0;

	while(totalRead == 0 || receivedSize != 0) {
		receivedSize = s2.receive(buf, sizeof(buf));
		if(receivedSize > -1) totalRead += receivedSize;
	}

	CHECK_EQUAL(13, totalRead);
	CHECK(strcmp("Hello world!", buf) == 0);
	thread.wait();
}

TEST_FIXTURE(BsdSocketTestFixture, Shutdown)
{
#ifndef MCD_IPHONE	// Has some problem on partial shutdown on iPhone
	BsdSocket sl, sa, sc;
	char buf[64];
	CHECK_EQUAL(0, setupConnectedTcpSockets(sl, sa, sc));

	CHECK_EQUAL(-1, sc.receive(buf, sizeof(buf)));

	// Server shutdown write, client should receive 0
	CHECK_EQUAL(0, sa.shutDownWrite());
	CHECK_EQUAL(-1, sa.send(buf, sizeof(buf)));

	ssize_t rec;
	while((rec = sc.receive(buf, sizeof(buf))) == -1) {}
	CHECK_EQUAL(0, rec);
	CHECK_EQUAL(0, sc.shutDownRead());

	// Client shutdown write, server should receive 0
	CHECK_EQUAL(0, sc.shutDownWrite());
	CHECK_EQUAL(-1, sc.send(buf, sizeof(buf)));

	while((rec = sa.receive(buf, sizeof(buf))) == -1) {}
	CHECK_EQUAL(0, rec);
	CHECK_EQUAL(0, sa.shutDownRead());
#endif
}

TEST_FIXTURE(BsdSocketTestFixture, Udp)
{
	BsdSocket s;
	CHECK_EQUAL(0, s.create(BsdSocket::UDP));
	CHECK_EQUAL(0, s.bind(mAnyEndPoint));

	BsdSocket s2;
	const IPEndPoint connectorEndPoint(IPAddress::getLoopBack(), 4321);	// Explicit end point for verification test
	CHECK_EQUAL(0, s2.create(BsdSocket::UDP));
	CHECK_EQUAL(0, s2.bind(connectorEndPoint));
	const char msg[] = "Hello world!";
	CHECK_EQUAL(ssize_t(sizeof(msg)), s2.sendTo(msg, sizeof(msg), mLocalEndPoint));

	char buf[64];
	IPEndPoint srcEndpoint(IPAddress::getAny(), 0);
	CHECK_EQUAL(ssize_t(sizeof(msg)), s.receiveFrom(buf, sizeof(buf), srcEndpoint));
	CHECK(srcEndpoint == connectorEndPoint);
}
