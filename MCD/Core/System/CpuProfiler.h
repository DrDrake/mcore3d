#ifndef __MCD_CORE_SYSTEM_CPUPROFILER__
#define __MCD_CORE_SYSTEM_CPUPROFILER__

#include "CallstackProfiler.h"
#include "Timer.h"
#include <string>

namespace MCD {

/*!	Used internally by CpuProfiler, but user may find it usefull if they need
	to generate customized report.
 */
class MCD_CORE_API CpuProfilerNode : public CallstackNode
{
public:
	CpuProfilerNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent=nullptr);

// Operations
	sal_override void begin();

	sal_override void end();

	sal_override CallstackNode* createNode(sal_in_z const char name[], sal_maybenull CallstackNode* parent);

	/*!	Reset the collected statistic including all child nodes.
		For saving memory, those call stack node with zero callCount will be deleted.
	 */
	void reset();

// Attributes
	//! Get the time spend of only this function (excluding any child call)
	float selfTime() const;

	size_t callCount;

	//! The time spend between the call of begin() and end()
	TimeInterval inclusiveTime;

	Timer timer;
};	// CpuProfilerNode

/*!	
 */
class MCD_CORE_API CpuProfiler : public CallstackProfiler
{
public:
	CpuProfiler();

	//! Handly class for scope profilinig
	class Scope : MCD::Noncopyable
	{
	public:
		Scope(const char name[]) {
			CpuProfiler::singleton().begin(name);
		}

		~Scope() {
			CpuProfiler::singleton().end();
		}
	};	// Scope

	static CpuProfiler& singleton();

// Operations
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

// Attributes
	//! Frame per second.
	float fps() const;

	size_t frameCount;	//! Number of frame elasped since last reset
	TimeInterval timeSinceLastReset;
	Timer timer;
};	// CpuProfiler

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_CPUPROFILER__
