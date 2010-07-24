#include "Pch.h"
#include "CpuProfiler.h"
#include <iomanip>
#include <sstream>

namespace MCD {

CpuProfilerNode::CpuProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent), callCount(0), inclusiveTime(uint64_t(0))
{}

void CpuProfilerNode::begin()
{
	// Start the timer for the first call, ignore all later recursive call
	if(recursionCount == 0)
		timer.reset();

	++callCount;
}

void CpuProfilerNode::end()
{
	if(recursionCount == 0)
		inclusiveTime += timer.get();
}

CallstackNode* CpuProfilerNode::createNode(const char name[], CallstackNode* parent)
{
	return new CpuProfilerNode(name, parent);
}

// Note that it takes a reference of a pointer
static void resetHelper(CallstackNode*& node)
{
	if(!node)
		return;

	// Free the node if it's not ever called during the last reset()
	if(static_cast<CpuProfilerNode*>(node)->callCount == 0) {
		delete node;
		node = nullptr;
	} else
		static_cast<CpuProfilerNode*>(node)->reset();
}

void CpuProfilerNode::reset()
{
	callCount = 0;
	inclusiveTime.set(uint64_t(0));

	resetHelper(firstChild);
	resetHelper(sibling);
}

float CpuProfilerNode::selfTime() const
{
	// Loop and sum for all direct children
	TimeInterval sum = 0;
	const CpuProfilerNode* n = static_cast<CpuProfilerNode*>(firstChild);
	while(n) {
		sum += n->inclusiveTime;
		n = static_cast<CpuProfilerNode*>(n->sibling);
	}

	return float((inclusiveTime - sum).asSecond());
}

CpuProfiler::CpuProfiler()
{
	setRootNode(new CpuProfilerNode("main root"));
}

CpuProfiler& CpuProfiler::singleton()
{
	static CpuProfiler instance;
	return instance;
}

void CpuProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
	reset();
}

void CpuProfiler::nextFrame()
{
	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call nextFrame() inside a profiling code block");
	++frameCount;
	timeSinceLastReset = timer.get();
}

void CpuProfiler::reset()
{
	if(!mRootNode)
		return;

	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call reset() inside a profiling code block");
	frameCount = 0;
	static_cast<CpuProfilerNode*>(mRootNode)->reset();
	timeSinceLastReset = TimeInterval::getMax();
	timer.reset();
}

float CpuProfiler::fps() const
{
	return float(frameCount / timeSinceLastReset.asSecond());
}

std::string CpuProfiler::defaultReport(size_t nameLength) const
{
	using namespace std;
	ostringstream ss;

	ss << "FPS: " << static_cast<size_t>(fps()) << endl;

	const size_t percentWidth = 8;
	const size_t floatWidth = 12;

	ss.flags(ios_base::left);
	ss	<< setw(nameLength)		<< "Name"
		<< setw(percentWidth)	<< "TT/F %"
		<< setw(percentWidth)	<< "ST/F %"
		<< setw(floatWidth)		<< "TT/C"
		<< setw(floatWidth)		<< "ST/C"
		<< setw(floatWidth)		<< "C/F"
		<< endl;

	CpuProfilerNode* n = static_cast<CpuProfilerNode*>(mRootNode);

	while((n = static_cast<CpuProfilerNode*>(CallstackNode::traverse(n))) != nullptr)
	{
		float percent = 100 * fps();
		float inclusiveTime = float(n->inclusiveTime.asSecond());

		// Skip node that have total time less than 1%
		if(inclusiveTime / frameCount * percent < 1)
			continue;

		float selfTime = n->selfTime();

		size_t callDepth = n->callDepth();
		ss	<< setw(callDepth - 1) << ""
			<< setw(nameLength - callDepth + 1) << n->name
			<< setprecision(3)
			<< setw(percentWidth)	<< (inclusiveTime / frameCount * percent)
			<< setw(percentWidth)	<< (selfTime / frameCount * percent)
			<< setw(floatWidth)		<< (inclusiveTime / n->callCount)
			<< setw(floatWidth)		<< (selfTime / n->callCount)
			<< setprecision(2)
			<< setw(floatWidth-2)	<< (float(n->callCount) / frameCount)
			<< endl;
	}

	return ss.str();
}

}	// namespace MCD
