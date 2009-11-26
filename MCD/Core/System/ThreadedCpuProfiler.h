#ifndef __MCD_CORE_SYSTEM_THREADEDCPUPROFILER__
#define __MCD_CORE_SYSTEM_THREADEDCPUPROFILER__

#include "CallstackProfiler.h"
#include "Mutex.h"
#include "Timer.h"
#include <string>

namespace MCD {

/*!	Used internally by ThreadedCpuProfiler, but user may find it usefull if they need
	to generate customized report.
 */
class MCD_CORE_API ThreadedCpuProfilerNode : public CallstackNode
{
public:
	ThreadedCpuProfilerNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent=nullptr);

// Operations
	sal_override void begin();

	sal_override void end();

	sal_override CallstackNode* createNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent);

	//!	Reset the collected statistic including all child nodes.
	void reset();

// Attributes
	//! Get the time spend of only this function (excluding any child call)
	float selfTime() const;

	size_t callCount;

	//! The time spend between the call of begin() and end()
	TimeInterval inclusiveTime;

	Timer timer;

	mutable RecursiveMutex mutex;
};	// ThreadedCpuProfilerNode

/*!	A multi-thread version of CpuProfiler.
	Since we need to access the profiler in multiple thread context, therefore
	this class is made as a singleton.
 */
class MCD_CORE_API ThreadedCpuProfiler : public CallstackProfiler
{
	ThreadedCpuProfiler();

	sal_override ~ThreadedCpuProfiler();

public:
		//! Handly class for scope profilinig
	class Scope : MCD::Noncopyable
	{
	public:
		Scope(const char name[]) {
			ThreadedCpuProfiler::singleton().begin(name);
		}

		~Scope() {
			ThreadedCpuProfiler::singleton().end();
		}
	};	// Scope

	static ThreadedCpuProfiler& singleton();

// Operations
	sal_override void begin(sal_in_z const char name[]);

	sal_override void end();

	void setRootNode(sal_maybenull CallstackNode* root);

	/*!	Inform the profiler a new iteration begins.
		This function is most likely to be called after each iteration of the main loop.
		\note Do not call this function between begin() and end()
	 */
	void nextFrame();

	/*!	Reset the state of the profiler.
		The profiler keeps calculating statistic for calculating average values later on.
		Therefore, if you want to take average over 10 frames, call this function every 10 frames.
		\note Do not call this function between begin() and end()
	 */
	void reset();

	std::string defaultReport(size_t nameLength=100) const;

	//!	Call this if you want to give your thread a meaningful name.
	void* onThreadAttach(sal_in_z const char* threadName = "worker thread");

// Attributes
	//! Frame per second.
	float fps() const;

	size_t frameCount;	//! Number of frame elasped since last reset
	TimeInterval timeSinceLastReset;
	Timer timer;

protected:
	struct TlsList;
	TlsList* mTlsList;
};	// ThreadedCpuProfiler

//! A TCP server for enabling external statistic report.
class MCD_CORE_API ThreadedCpuProfilerServer
{
public:
	ThreadedCpuProfilerServer();

	~ThreadedCpuProfilerServer();

	bool listern(uint16_t port);

	bool accept();

	/*!	Flush the report to the client (if connected) and reset the statistic.
		This function is supposed to be called every 0.5 to few seconds.
	 */
	void update();

protected:
	class Impl;
	Impl& mImpl;
};	// ThreadedCpuProfilerServer

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_THREADEDCPUPROFILER__
