#include "Pch.h"
#include "MemoryProfiler.h"

#if defined(_MSC_VER) && !defined(NDEBUG)

#include <iomanip>
#include <sstream>
#include <crtdbg.h>
#include "dbgint.h"

#include "FunctionPatcher.inc"

struct AllocInfo {
	size_t count;
	size_t bytes;
};

typedef void* (__cdecl *MyMalloc)(size_t);
typedef void* (__cdecl *MyMallocDbg)(size_t, int, const char*, int);
typedef void* (__cdecl *MyCMallocDbg)(size_t, size_t, int, const char*, int);
typedef void* (__cdecl *MyReallocDbg)(void*, size_t, int, const char*, int);
typedef void (__cdecl *MyFreeDbg)(void*, int);

void* __cdecl myMalloc(size_t);
void* __cdecl myMallocDbg(size_t, int, const char*, int);
void* __cdecl myCMallocDbg(size_t, size_t, int, const char*, int);
void* __cdecl myReallocDbg(void*, size_t, int, const char*, int);
void __cdecl myFreeDbg(void*, int);

FunctionPatcher functionPatcher;
MyMalloc orgMalloc;
MyMallocDbg orgMallocDbg;
MyCMallocDbg orgCMallocDbg;
MyReallocDbg orgReallocDbg;
MyFreeDbg orgFreeDbg;

/*!	Record the first request number that our profiler encounter, so that we
	can ignore those previous allocations in our deallocation hook.
 */
static long gFirstRequestNumber = LONG_MAX;
static AllocInfo gAlloc;
static AllocInfo gAccumAlloc;

void* commonAlloc(void* p, size_t nBytes)
{
	_CrtMemBlockHeader* header = pHdr(p);

	if(gFirstRequestNumber == LONG_MAX)
		gFirstRequestNumber = header->lRequest;

	gAlloc.count++;
	gAlloc.bytes += nBytes;
	gAccumAlloc.count++;
	gAccumAlloc.bytes += nBytes;

	MCD::MemoryProfilerNode* node = static_cast<MCD::MemoryProfilerNode*>(
		MCD::MemoryProfiler::singleton().getCurrentNode());

	node->exclusiveCount++;
	node->exclusiveBytes += nBytes;
	node->countSinceLastReset++;

	header->szFileName = reinterpret_cast<char*>(node);

	return p;
}

void* __cdecl myMalloc(size_t nBytes)
{
	void* p = orgMalloc(nBytes);
	return commonAlloc(p, nBytes);
}

void* __cdecl myMallocDbg(size_t nBytes, int nBlockUse, const char* szFileName, int nLine)
{
	void* p = orgMallocDbg(nBytes, nBlockUse, szFileName, nLine);
	return commonAlloc(p, nBytes);
}

void* __cdecl myCMallocDbg(size_t nNum, size_t nBytes, int nBlockUse, const char* szFileName, int nLine)
{
	void* p = orgCMallocDbg(nNum, nBytes, nBlockUse, szFileName, nLine);
	return commonAlloc(p, nNum * nBytes);
}

void* __cdecl myReallocDbg(void* original, size_t nBytes, int nBlockUse, const char* szFileName, int nLine)
{
	size_t originalSize = 0;

	if(original) {
		_CrtMemBlockHeader* header = pHdr(original);
		originalSize = header->nDataSize;
	}

	// NOTE: Calling orgReallocDbg() may in turn invoke myMallocDbg() or myFreeDbg()
	void* p = orgReallocDbg(original, nBytes, nBlockUse, szFileName, nLine);
	if(original == NULL || nBytes == 0)
		return p;

	MCD_ASSERT(gFirstRequestNumber != LONG_MAX);

	nBytes -= originalSize;
	MCD::MemoryProfilerNode* node = static_cast<MCD::MemoryProfilerNode*>(
		MCD::MemoryProfiler::singleton().getCurrentNode());

	node->exclusiveBytes += nBytes;
	gAlloc.bytes += nBytes;
	gAccumAlloc.bytes += nBytes;

	_CrtMemBlockHeader* header = pHdr(p);
	header->szFileName = reinterpret_cast<char*>(node);

	return p;
}

void __cdecl myFreeDbg(void* p, int nBlockUse)
{
	if(p)
	{
		_CrtMemBlockHeader* header = pHdr(p);
		if(header->lRequest >= gFirstRequestNumber)
		{
			// NOTE: This the point of danger.
			MCD::MemoryProfilerNode* node = reinterpret_cast<MCD::MemoryProfilerNode*>(header->szFileName);
			MCD_ASSUME(node && "Some allocation functions not patched!");
			node->exclusiveCount--;
			node->exclusiveBytes -= header->nDataSize;
			gAlloc.count--;
			gAlloc.bytes -= header->nDataSize;
		}
	}

	return orgFreeDbg(p, nBlockUse);
}

namespace MCD {

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	:
	CallstackNode(name, parent), callCount(0),
	exclusiveCount(0), exclusiveBytes(0), countSinceLastReset(0)
{
}

void MemoryProfilerNode::begin()
{
	++callCount;
}

void MemoryProfilerNode::end()
{
}

// Note that it takes a reference of a pointer
static void resetHelper(CallstackNode*& node)
{
	if(!node)
		return;

	// Free the node if there is no associated allocation
	if(static_cast<MemoryProfilerNode*>(node)->exclusiveCount == 0) {
		delete node;
		node = nullptr;
	} else
		static_cast<MemoryProfilerNode*>(node)->reset();
}

void MemoryProfilerNode::reset()
{
	callCount = 0;
	countSinceLastReset = 0;

	resetHelper(firstChild);
	resetHelper(sibling);
}

size_t MemoryProfilerNode::inclusiveCount() const
{
	size_t total = exclusiveCount;
	const MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(firstChild);
	if(!n)
		return total;

	do {
		total += n->inclusiveCount();
		n = static_cast<MemoryProfilerNode*>(n->sibling);
	} while(n);

	return total;
}

size_t MemoryProfilerNode::inclusiveBytes() const
{
	size_t total = exclusiveBytes;
	const MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(firstChild);
	if(!n)
		return total;

	do {
		total += n->inclusiveBytes();
		n = static_cast<MemoryProfilerNode*>(n->sibling);
	} while(n);

	return total;
}

MemoryProfiler::MemoryProfiler()
{
	// Back up the original function and then do patching
	orgMalloc		= (MyMalloc) functionPatcher.copyPrologue(&malloc);
	orgMallocDbg	= (MyMallocDbg) functionPatcher.copyPrologue(&_malloc_dbg);
	orgCMallocDbg	= (MyCMallocDbg) functionPatcher.copyPrologue(&_calloc_dbg);
	orgReallocDbg	= (MyReallocDbg) functionPatcher.copyPrologue(&_realloc_dbg);
	orgFreeDbg		= (MyFreeDbg) functionPatcher.copyPrologue(&_free_dbg);

	functionPatcher.patch(&malloc, &myMalloc);
	functionPatcher.patch(&_malloc_dbg, &myMallocDbg);
	functionPatcher.patch(&_calloc_dbg, &myCMallocDbg);
	functionPatcher.patch(&_realloc_dbg, &myReallocDbg);
	functionPatcher.patch(&_free_dbg, &myFreeDbg);
}

MemoryProfiler::~MemoryProfiler()
{
	functionPatcher.UnpatchAll();
}

void MemoryProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
	reset();
}

void MemoryProfiler::nextFrame()
{
	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call nextFrame() inside a profiling code block");
	++frameCount;
}

void MemoryProfiler::reset()
{
	if(!mRootNode)
		return;

	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call reset() inside a profiling code block");
	frameCount = 0;
	static_cast<MemoryProfilerNode*>(mRootNode)->reset();
}

std::string MemoryProfiler::defaultReport(size_t nameLength) const
{
	using namespace std;
	ostringstream ss;

	ss.flags(ios_base::left);
	ss << setw(30) << "Allocated count: " << setw(10) << gAlloc.count << ", bytes: " << gAlloc.bytes << endl;
	ss << setw(30) << "Accumulated allocated count: " << setw(10) << gAccumAlloc.count << ", bytes: " << gAccumAlloc.bytes << endl;

	const size_t countWidth = 9;
	const size_t bytesWidth = 12;

	ss	<< setw(nameLength)		<< "Name"
		<< setw(countWidth)		<< "TCount"
		<< setw(countWidth)		<< "SCount"
		<< setw(bytesWidth)		<< "TBytes"
		<< setw(bytesWidth)		<< "SBytes"
		<< setw(countWidth)		<< "TCount/F"
		<< setw(countWidth)		<< "Call/F"
		<< endl;

	MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(mRootNode);

	while((n = static_cast<MemoryProfilerNode*>(CallstackNode::traverse(n))) != nullptr)
	{
		// Skip node that have no allocation at all
		if(n->exclusiveCount == 0 && n->countSinceLastReset == 0)
			continue;

		size_t callDepth = n->callDepth();
		ss	<< setw(callDepth - 1) << ""
			<< setw(nameLength - callDepth + 1) << n->name
			<< setprecision(3)
			<< setw(countWidth)		<< (n->inclusiveCount())
			<< setw(countWidth)		<< (n->exclusiveCount)
			<< setw(bytesWidth)		<< (n->inclusiveBytes())
			<< setw(bytesWidth)		<< (n->exclusiveBytes)
			<< setw(countWidth)		<< (float(n->countSinceLastReset) / frameCount)
			<< setprecision(2)
			<< setw(countWidth-2)	<< (float(n->callCount) / 1)
			<< endl;
	}

	return ss.str();
}

}	// namespace MCD

#else

namespace MCD {

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent)
{}

void MemoryProfilerNode::begin() {}

void MemoryProfilerNode::end() {}

MemoryProfiler::MemoryProfiler() {}

MemoryProfiler::~MemoryProfiler() {}

void MemoryProfiler::setRootNode(CallstackNode* root) {
	CallstackProfiler::setRootNode(root);
}

void MemoryProfiler::nextFrame() {}

void MemoryProfiler::reset() {}

std::string MemoryProfiler::defaultReport(size_t nameLength) const {
	return std::string();
}

}	// namespace MCD

#endif	// _MSC_VER

MCD::CallstackNode* MCD::MemoryProfilerNode::createNode(const char name[], MCD::CallstackNode* parent)
{
	return new MCD::MemoryProfilerNode(name, parent);
}

MCD::MemoryProfiler& MCD::MemoryProfiler::singleton()
{
	static MCD::MemoryProfiler instance;
	return instance;
}
