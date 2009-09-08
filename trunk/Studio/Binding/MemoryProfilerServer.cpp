#include "stdafx.h"
#include "MemoryProfilerServer.h"

namespace Binding {

MemoryProfilerServer::MemoryProfilerServer()
{
	mImpl = new MCD::MemoryProfilerServer;
	MCD::MemoryProfiler::singleton().setEnable(true);
}

MemoryProfilerServer::~MemoryProfilerServer()
{
	this->!MemoryProfilerServer();
}

MemoryProfilerServer::!MemoryProfilerServer()
{
	delete mImpl;
}

bool MemoryProfilerServer::listern(uint16_t port)
{
	return mImpl->listern(port);
}

bool MemoryProfilerServer::accept()
{
	return mImpl->accept();
}

void MemoryProfilerServer::update()
{
	accept();
	mImpl->update();
}

}	// namespace Binding
