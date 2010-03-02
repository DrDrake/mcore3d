#include "Pch.h"
#include "../../MCD/Network/BsdSocket.h"

using namespace MCD;

namespace {

class BsdSocketTestFixture
{
protected:
	BsdSocketTestFixture()
	{
		BsdSocket::initApplication();
	}

	~BsdSocketTestFixture()
	{
		BsdSocket::closeApplication();
	}
};	// BsdSocketTestFixture

}	// namespace

TEST_FIXTURE(BsdSocketTestFixture, Basic)
{
	BsdSocket s1;
	s1.create(BsdSocket::TCP);
}
