#include "Pch.h"
#include "CpuProfiler.h"
#include "PlatformInclude.h"
#include "PtrVector.h"
#include <iomanip>
#include <sstream>

namespace MCD {

namespace {

//! A structure that group all global varaibles into a thread local storage.
struct TlsStruct
{
	TlsStruct(const char* name) :
		recurseCount(0),
		// We want every thread have it's own copy of thread name, therefore strdup() is used
		mCurrentNode(nullptr), threadName(::strdup(name))
	{}

	~TlsStruct() {
		::free((void*)threadName);
	}

	sal_notnull CpuProfilerNode* currentNode()
	{
		// If node is null, means a new thread is started
		if(!mCurrentNode) {
			CallstackNode* rootNode = CpuProfiler::singleton().getRootNode();
			MCD_ASSUME(rootNode);
			recurseCount++;
			mCurrentNode = static_cast<CpuProfilerNode*>(
				rootNode->getChildByName(threadName)
			);
			recurseCount--;
		}

		return mCurrentNode;
	}

	CpuProfilerNode* setCurrentNode(CallstackNode* node) {
		return mCurrentNode = static_cast<CpuProfilerNode*>(node);
	}

	size_t recurseCount;
	const char* threadName;

protected:
	CpuProfilerNode* mCurrentNode;
};	// TlsStruct

DWORD gTlsIndex = 0;

TlsStruct* getTlsStruct()
{
	MCD_ASSUME(gTlsIndex != 0);
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

}	// namespace

struct CpuProfiler::TlsList : public ptr_vector<TlsStruct>
{
	Mutex mutex;
};	// TlsList

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

void CpuProfilerNode::reset()
{
	CpuProfilerNode* n1, *n2;
	{	// Race with CpuProfiler::begin(), CpuProfiler::end()
		ScopeRecursiveLock lock(mutex);
		callCount = 0;
		inclusiveTime.set(uint64_t(0));
		n1 = static_cast<CpuProfilerNode*>(firstChild);
		n2 = static_cast<CpuProfilerNode*>(sibling);
	}

	if(n1) n1->reset();
	if(n2) n2->reset();
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
	mTlsList = new TlsList();
	gTlsIndex = TlsAlloc();
	setRootNode(new CpuProfilerNode("root"));

	onThreadAttach("main thread");
}

CpuProfiler::~CpuProfiler()
{
	MCD_ASSERT(gTlsIndex != 0);
	TlsSetValue(gTlsIndex, nullptr);
	TlsFree(gTlsIndex);
	gTlsIndex = 0;

	// We assume that all thread will be stopped before CpuProfiler is destroyed
	delete mTlsList;
}

CpuProfiler& CpuProfiler::singleton()
{
	static CpuProfiler instance;
	return instance;
}

void CpuProfiler::begin(const char name[])
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	if(!tls)
		tls = reinterpret_cast<TlsStruct*>(onThreadAttach("worker thread"));
	CpuProfilerNode* node = tls->currentNode();

	// Race with CpuProfiler::reset(), CpuProfiler::defaultReport()
	ScopeRecursiveLock lock(node->mutex);

	if(name != node->name) {
		node = static_cast<CpuProfilerNode*>(node->getChildByName(name));

		// Only alter the current node, if the child node is not recursing
		if(node->recursionCount == 0)
			tls->setCurrentNode(node);
	}

	node->begin();
	node->recursionCount++;
}

void CpuProfiler::end()
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	CpuProfilerNode* node = tls->currentNode();

	// Race with CpuProfiler::reset(), CpuProfiler::defaultReport()
	ScopeRecursiveLock lock(node->mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->setCurrentNode(node->parent);
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

	do
	{
		// Race with CpuProfiler::begin() and CpuProfiler::end()
		ScopeRecursiveLock lock(n->mutex);

		float percent = 100 * fps();
		float inclusiveTime = float(n->inclusiveTime.asSecond());

		// Skip node that have total time less than 1%
//		if(inclusiveTime / frameCount * percent < 1)
//			continue;

		float selfTime = n->selfTime();

		size_t callDepth = n->callDepth();
		ss	<< setw(callDepth) << ""
			<< setw(nameLength - callDepth + 1) << n->name
			<< setprecision(3)
			<< setw(percentWidth)	<< (inclusiveTime / frameCount * percent)
			<< setw(percentWidth)	<< (selfTime / frameCount * percent)
			<< setw(floatWidth)		<< (n->callCount == 0 ? 0 : inclusiveTime / n->callCount)
			<< setw(floatWidth)		<< (n->callCount == 0 ? 0 : selfTime / n->callCount)
			<< setprecision(2)
			<< setw(floatWidth-2)	<< (float(n->callCount) / frameCount)
			<< endl;

		n = static_cast<CpuProfilerNode*>(CallstackNode::traverse(n));
	} while(n != nullptr);

	return ss.str();
}

void* CpuProfiler::onThreadAttach(const char* threadName)
{
	TlsStruct* tls = new TlsStruct(threadName);

	{
		ScopeLock lock(mTlsList->mutex);
		mTlsList->push_back(tls);
	}

	TlsSetValue(gTlsIndex, tls);

	return tls;
}

}	// namespace MCD
