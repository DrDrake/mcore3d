#include "stdafx.h"
#include "ProfilerServer.h"

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

void MemoryProfilerServer::flush()
{
	accept();
	mImpl->flush();
}

CpuProfilerServer::CpuProfilerServer()
{
	mImpl = new MCD::ThreadedCpuProfilerServer;
	MCD::ThreadedCpuProfiler::singleton().enable = true;
}

CpuProfilerServer::~CpuProfilerServer()
{
	this->!CpuProfilerServer();
}

CpuProfilerServer::!CpuProfilerServer()
{
	delete mImpl;
}

bool CpuProfilerServer::listern(uint16_t port)
{
	return mImpl->listern(port);
}

bool CpuProfilerServer::accept()
{
	return mImpl->accept();
}

void CpuProfilerServer::flush()
{
	accept();
	mImpl->flush();
}

}	// namespace Binding
