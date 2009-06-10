#ifndef __MCD_CORE_SYSTEM_MEMORYPROFILER__
#define __MCD_CORE_SYSTEM_MEMORYPROFILER__

#include "CallstackProfiler.h"
#include "Mutex.h"

namespace MCD {

/*!	
	There is also a very good article about memory allocator, profiling etc
	http://entland.homelinux.com/blog/2008/08/19/practical-efficient-memory-management/
 */
class MCD_CORE_API MemoryProfilerNode : public CallstackNode
{
public:
	MemoryProfilerNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent=nullptr);

	sal_override ~MemoryProfilerNode();

// Operations
	sal_override void begin();

	sal_override CallstackNode* createNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent);

	/*!	Reset the collected statistic including all child nodes.
		For saving memory, those call stack node with zero allocatino count will be deleted.
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
	mutable RecursiveMutex mutex;

	/*!	Since each thread should have it's own node's name as the thread's
		root node, we use a boolean flag to indicate the \em name variable
		is not a static const and so need to free.
	 */
	bool shouldFreeNodeName;
};	// MemoryProfilerNode

/*!	The memory profiler will hook the various memory routine in
	the C runtime to do profiling, and so multiple instacne of it
	is not allowed.
 */
class MCD_CORE_API MemoryProfiler : public CallstackProfiler
{
	MemoryProfiler();

	~MemoryProfiler();

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

// Attributes
	size_t frameCount;	//! Number of frame elasped since last reset
};	// MemoryProfiler

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_MEMORYPROFILER__
