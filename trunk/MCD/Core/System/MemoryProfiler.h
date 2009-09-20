#ifndef __MCD_CORE_SYSTEM_MEMORYPROFILER__
#define __MCD_CORE_SYSTEM_MEMORYPROFILER__

#include "CallstackProfiler.h"
#include "Mutex.h"
#include <string>

namespace MCD {

/*!	A call stack node that store memory usage information for individual function.
	Used internally by MemoryProfiler, but user may find it usefull if they need
	to generate customized report.
 */
class MCD_CORE_API MemoryProfilerNode : public CallstackNode
{
public:
	MemoryProfilerNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent=nullptr);

	~MemoryProfilerNode();

// Operations
	sal_override void begin();

	sal_override CallstackNode* createNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent);

	/*!	Reset the collected statistic including all child nodes.
		This function is usefull for a CPU profiler but not so meaningfull for a memory profiler.
	 */
	void reset();

// Attributes
	//! Get the memory allocation count within this function (including child call)
	size_t inclusiveCount() const;

	//! Get the memory allocated (in bytes) within this function (including child call)
	size_t inclusiveBytes() const;

	size_t callCount;

	int exclusiveCount;
	int exclusiveBytes;

	/*!	Number of allocation made since last reset, of course
		you want to minimize this figure to almost no allocation at
		all in each frame.
	 */
	size_t countSinceLastReset;

	/*!	The memory profiler will utilize TLS (thread local storage) as much as
		possible, but in some cases (eg. MemoryProfiler::reset and MemoryProfiler::defaultReport)
		locking is still needed.
	 */
	bool mIsMutexOwner;
	RecursiveMutex* mutex;
};	// MemoryProfilerNode

/*!	A memory profiler.

	The memory profiler will hook the various memory routine in
	the C runtime to do profiling, and so multiple instacne of it
	is not allowed.

	There is also a very good article about memory allocator, profiling etc
	http://entland.homelinux.com/blog/2008/08/19/practical-efficient-memory-management/
 */
class MCD_CORE_API MemoryProfiler : public CallstackProfiler
{
	MemoryProfiler();

	sal_override ~MemoryProfiler();

public:
	//! Handly class for scope profilinig
	class Scope : MCD::Noncopyable
	{
	public:
		Scope(const char name[]) {
			MemoryProfiler::singleton().begin(name);
		}

		~Scope() {
			MemoryProfiler::singleton().end();
		}
	};	// Scope

	static MemoryProfiler& singleton();

// Operations
	void setRootNode(sal_maybenull CallstackNode* root);

	sal_override void begin(const char name[]);

	sal_override void end();

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
	void* onThreadAttach(sal_in_z const char* threadName = "WORKER THREAD");

// Attributes
	bool enable() const;

	void setEnable(bool flag);

	size_t frameCount;	//! Number of frame elasped since last reset

protected:
	struct TlsList;
	TlsList* mTlsList;
};	// MemoryProfiler

//! A TCP server for enabling external statisstic report.
class MCD_CORE_API MemoryProfilerServer
{
public:
	MemoryProfilerServer();

	~MemoryProfilerServer();

	bool listern(uint16_t port);

	bool accept();

	/*!	Flush the report to the client (if connected) and reset the statistic.
		This function is supposed to be called every 0.5 to few seconds.
	 */
	void update();

protected:
	class Impl;
	Impl* mImpl;
};	// MemoryProfilerServer

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_MEMORYPROFILER__
