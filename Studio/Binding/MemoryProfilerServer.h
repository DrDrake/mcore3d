#pragma once

#include "../../MCD/Core/System/MemoryProfiler.h"
#include "../../MCD/Core/System/ThreadedCpuProfiler.h"
#undef nullptr

namespace Binding {

public ref class MemoryProfilerServer
{
public:
	MemoryProfilerServer();

	bool listern(uint16_t port);

	bool accept();

	void update();

protected:
	~MemoryProfilerServer();
	!MemoryProfilerServer();

	MCD::MemoryProfilerServer* mImpl;
};	// MemoryProfilerServer

public ref class CpuProfilerServer
{
public:
	CpuProfilerServer();

	bool listern(uint16_t port);

	bool accept();

	void update();

protected:
	~CpuProfilerServer();
	!CpuProfilerServer();

	MCD::ThreadedCpuProfilerServer* mImpl;
};	// CpuProfilerServer

}	// namespace Binding
