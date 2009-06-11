#include "Pch.h"
#include "MemoryProfiler.h"

#if defined(_MSC_VER)

#include "FunctionPatcher.inc"
#include <iomanip>
#include <sstream>

using namespace MCD;

namespace {

struct AllocInfo
{
	AllocInfo() : count(0), bytes(0) {}
	size_t count;
	size_t bytes;
};	// AllocInfo

//! A structure that group all global varaibles into a thread local storage.
struct TlsStruct
{
	TlsStruct(const char* name) :
		gTempDisable(false), gRecurseCount(0),
		gCurrentNode(nullptr), threadName(::strdup(name))
	{}

	~TlsStruct() {
		::free((void*)threadName);
	}

	AllocInfo gAlloc;
	AllocInfo gAccumAlloc;
	bool gTempDisable;
	size_t gRecurseCount;
	MemoryProfilerNode* gCurrentNode;
	const char* threadName;
};	// TlsStruct

typedef LPVOID (WINAPI *MyHeapAlloc)(HANDLE, DWORD, SIZE_T);
typedef LPVOID (WINAPI *MyHeapReAlloc)(HANDLE, DWORD, LPVOID, SIZE_T);
typedef LPVOID (WINAPI *MyHeapFree)(HANDLE, DWORD, LPVOID);

LPVOID WINAPI myHeapAlloc(HANDLE, DWORD, SIZE_T);
LPVOID WINAPI myHeapReAlloc(HANDLE, DWORD, LPVOID, SIZE_T);
LPVOID WINAPI myHeapFree(HANDLE, DWORD, LPVOID);

FunctionPatcher functionPatcher;
MyHeapAlloc orgHeapAlloc;
MyHeapReAlloc orgHeapReAlloc;
MyHeapFree orgHeapFree;

DWORD gTlsIndex = 0;

TlsStruct* getTlsStruct() {
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

struct MyMemFooter
{
	MCD::MemoryProfilerNode* node;
	uint32_t fourCC;
	//! A magic number for HeapFree verification
	static const uint32_t cFourCC = 1234567890;
};	// MyMemFooter

/*!	nBytes does not account for the extra footer size
	deltaBytes is non-zero when commonAlloc is used for realloc
 */
void* commonAlloc(sal_in TlsStruct* tls, sal_in void* p, size_t nBytes, int deltaBytes)
{
	size_t bytes = deltaBytes != 0 ? deltaBytes : nBytes;

	MCD_ASSUME(tls && p && "caller of commonAlloc should ensure tls and p is valid");
	tls->gAlloc.count++;
	tls->gAlloc.bytes += bytes;
	tls->gAccumAlloc.count++;
	tls->gAccumAlloc.bytes += bytes;

	MemoryProfilerNode* node = tls->gCurrentNode;

	// If node is null, means a new thread is started
	if(!node) {
		tls->gTempDisable = true;
		tls->gCurrentNode = node = static_cast<MCD::MemoryProfilerNode*>(
			MemoryProfiler::singleton().getRootNode()->getChildByName(tls->threadName)
		);
		node->shouldFreeNodeName = true;	// Pass the ownership of tls->threadName to node->name
		tls->threadName = nullptr;			//
		tls->gTempDisable = false;
	}

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);
	if(deltaBytes == 0) {
		node->exclusiveCount++;
		node->countSinceLastReset++;
	}
	node->exclusiveBytes += bytes;

	MyMemFooter* footer = reinterpret_cast<MyMemFooter*>(nBytes + (char*)p);
	footer->node = node;
	footer->fourCC = MyMemFooter::cFourCC;

	return p;
}

LPVOID WINAPI myHeapAlloc(__in HANDLE hHeap, __in DWORD dwFlags, __in SIZE_T dwBytes)
{
	TlsStruct* tls = getTlsStruct();

	// HeapAlloc will invoke itself recursivly, so we need a recursion counter
	if(!tls || tls->gRecurseCount > 0 || tls->gTempDisable)
		return orgHeapAlloc(hHeap, dwFlags, dwBytes);

	tls->gRecurseCount++;
	void* p = orgHeapAlloc(hHeap, dwFlags, dwBytes + sizeof(MyMemFooter));
	tls->gRecurseCount--;

	return commonAlloc(tls, p, dwBytes, 0);
}

LPVOID WINAPI myHeapReAlloc(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem, __in SIZE_T dwBytes)
{
	TlsStruct* tls = getTlsStruct();

	if(!tls || tls->gRecurseCount > 0 || lpMem == nullptr || dwBytes == 0)
		return orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);

	size_t size = HeapSize(hHeap, dwFlags, lpMem);

	// On VC 2005, orgHeapReAlloc() will not invoke HeapAlloc() and HeapFree(),
	// but it does on VC 2008
	tls->gRecurseCount++;
	void* p = orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes + sizeof(MyMemFooter));
	tls->gRecurseCount--;

	int deltaSize = (dwBytes + sizeof(MyMemFooter)) - size;

	return commonAlloc(tls, p, dwBytes, deltaSize);
}

LPVOID WINAPI myHeapFree(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem)
{
	TlsStruct* tls = getTlsStruct();
	// NOTE: For VC 2008, myHeapFree may be invoked by HeapRealloc,
	// therefore we need to check gRecurseCount
	if(lpMem && tls && tls->gRecurseCount == 0)
	{
		size_t size = HeapSize(hHeap, dwFlags, lpMem) - sizeof(MyMemFooter);

		MyMemFooter* footer = (MyMemFooter*)(((char*)lpMem) + size);
		if(footer->fourCC == MyMemFooter::cFourCC)
		{
			MCD::MemoryProfilerNode* node = reinterpret_cast<MCD::MemoryProfilerNode*>(footer->node);
			MCD_ASSUME(node);

			// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and all other
			// operations if lpMem is allocated from thread A but now free in thread B (this thread)
			ScopeRecursiveLock lock(node->mutex);
			node->exclusiveCount--;
			node->exclusiveBytes -= size;
			tls->gAlloc.count--;
			tls->gAlloc.bytes -= size;
		}
	}

	return orgHeapFree(hHeap, dwFlags, lpMem);
}

}	// namespace

namespace MCD {

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	:
	CallstackNode(name, parent), callCount(0),
	exclusiveCount(0), exclusiveBytes(0), countSinceLastReset(0),
	shouldFreeNodeName(false)
{
}

MemoryProfilerNode::~MemoryProfilerNode()
{
	if(shouldFreeNodeName)
		::free((void*)name);
}

void MemoryProfilerNode::begin()
{
	++callCount;
}

void resetHelper(CallstackNode* node)
{
	MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(node);
	if(!n)
		return;

	// Free the node if there is no associated allocation
//	if(n->exclusiveCount == 0) {
//		delete node;
//		node = nullptr;
//	} else
		n->reset();
}

void MemoryProfilerNode::reset()
{
	CallstackNode* n1, *n2;
	{	// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and myHeapFree()
		ScopeRecursiveLock lock(mutex);
		callCount = 0;
		countSinceLastReset = 0;
		n1 = firstChild;
		n2 = sibling;
	}

	resetHelper(n1);
	resetHelper(n2);
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
	setRootNode(new MemoryProfilerNode("main root"));

	// Pre-computed prologue size (for different version of Visual Studio) using libdasm
#if _MSC_VER == 1400	// VC 2005
	const int prologueSize[] = { 5, 5, 5 };
#else _MSC_VER > 1400	// VC 2008
	const int prologueSize[] = { 5, 5 ,5 };
#endif

	// Back up the original function and then do patching
	orgHeapAlloc	= (MyHeapAlloc) functionPatcher.copyPrologue(&HeapAlloc, prologueSize[0]);
	orgHeapReAlloc	= (MyHeapReAlloc) functionPatcher.copyPrologue(&HeapReAlloc, prologueSize[1]);
	orgHeapFree		= (MyHeapFree) functionPatcher.copyPrologue(&HeapFree, prologueSize[2]);

	functionPatcher.patch(&HeapAlloc, &myHeapAlloc);
	functionPatcher.patch(&HeapReAlloc, &myHeapReAlloc);
	functionPatcher.patch(&HeapFree, &myHeapFree);
}

MemoryProfiler::~MemoryProfiler()
{
	functionPatcher.UnpatchAll();
}

void MemoryProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
	TlsStruct* tls = getTlsStruct();
	tls->gCurrentNode = static_cast<MemoryProfilerNode*>(root);
	reset();
}

void MemoryProfiler::begin(const char name[])
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	MemoryProfilerNode* node = static_cast<MemoryProfilerNode*>(tls->gCurrentNode);
	MCD_ASSUME(node);

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	if(name != node->name)
		node = tls->gCurrentNode = static_cast<MemoryProfilerNode*>(node->getChildByName(name));

	node->begin();
	node->recursionCount++;
}

void MemoryProfiler::end()
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	MemoryProfilerNode* node = static_cast<MemoryProfilerNode*>(tls->gCurrentNode);
	MCD_ASSUME(node);

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->gCurrentNode = static_cast<MemoryProfilerNode*>(node->parent);
}

void MemoryProfiler::nextFrame()
{
	MCD_ASSERT(getTlsStruct()->gCurrentNode == mRootNode && "Do not call nextFrame() inside a profiling code block");
	++frameCount;
}

void MemoryProfiler::reset()
{
	if(!mRootNode)
		return;

	MCD_ASSERT(getTlsStruct()->gCurrentNode == mRootNode && "Do not call reset() inside a profiling code block");
	frameCount = 0;
	static_cast<MemoryProfilerNode*>(mRootNode)->reset();
}

std::string MemoryProfiler::defaultReport(size_t nameLength) const
{
	using namespace std;
	ostringstream ss;

	ss.flags(ios_base::left);
//	ss << setw(30) << "Allocated count: " << setw(10) << gAlloc.count << ", kBytes: " << float(gAlloc.bytes) / 1024 << endl;
//	ss << setw(30) << "Accumulated allocated count: " << setw(10) << gAccumAlloc.count << ", kBytes: " << float(gAccumAlloc.bytes) / 1024 << endl;

	const size_t countWidth = 9;
	const size_t bytesWidth = 12;

	ss.flags(ios_base::left);
	ss	<< setw(nameLength)		<< "Name" << setiosflags(ios::right)
		<< setw(countWidth)		<< "TCount"
		<< setw(countWidth)		<< "SCount"
		<< setw(bytesWidth)		<< "TkBytes"
		<< setw(bytesWidth)		<< "SkBytes"
		<< setw(countWidth)		<< "SCount/F"
		<< setw(countWidth)		<< "Call/F"
		<< endl;

	MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(mRootNode);

	do
	{	// NOTE: The following std stream operation may trigger HeapAlloc,
		// there we need to use recursive mutex here.

		// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and myHeapFree()
		ScopeRecursiveLock lock(n->mutex);

		// Skip node that have no allocation at all
		if(n->exclusiveCount != 0 || n->countSinceLastReset != 0)
		{
			size_t callDepth = n->callDepth();
			ss.flags(ios_base::left);
			ss	<< setw(callDepth) << ""
				<< setw(nameLength - callDepth) << n->name
				<< setiosflags(ios::right)// << setprecision(3)
				<< setw(countWidth)		<< (n->inclusiveCount())
				<< setw(countWidth)		<< (n->exclusiveCount)
				<< setw(bytesWidth)		<< (float(n->inclusiveBytes()) / 1024)
				<< setw(bytesWidth)		<< (float(n->exclusiveBytes) / 1024)
				<< setw(countWidth)		<< (float(n->countSinceLastReset) / frameCount)
				<< setprecision(2)
				<< setw(countWidth-2)	<< (float(n->callCount) / frameCount)
				<< endl;
		}

		n = static_cast<MemoryProfilerNode*>(CallstackNode::traverse(n));
	} while(n != nullptr);

	return ss.str();
}

}	// namespace MCD

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
	TlsStruct* tls = nullptr;

	switch(dwReason) {
	case DLL_PROCESS_ATTACH:
		gTlsIndex = TlsAlloc();
		TlsSetValue(gTlsIndex, new TlsStruct(nullptr));
		MemoryProfiler::singleton();
		break;
	case DLL_PROCESS_DETACH:
		tls = getTlsStruct();
		delete tls;
		TlsFree(gTlsIndex);
		break;
	case DLL_THREAD_ATTACH:
		// NOTE: Allocation of TlsStruct didn't trigger commonAlloc() since we
		// haven't called TlsSetValue() yet and so myHeapAlloc will by pass it.
		TlsSetValue(gTlsIndex, new TlsStruct("worker thread"));
		break;
	// TODO: DLL_THREAD_DETACH is not exactly paried with DLL_THREAD_ATTACH, causing memory leak.
	case DLL_THREAD_DETACH:
		tls = getTlsStruct();
		delete tls;
		break;
	default:
		break;
	}
	return TRUE;
}

#else

namespace MCD {

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent)
{}

MemoryProfilerNode::~MemoryProfilerNode() {}

void MemoryProfilerNode::begin() {}

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
