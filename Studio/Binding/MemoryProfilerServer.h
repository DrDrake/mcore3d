#pragma once

#include "../../MCD/Core/System/MemoryProfiler.h"
#undef nullptr

namespace Binding {

/*!	A place for everyone to access
 */
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

}	// namespace Binding
