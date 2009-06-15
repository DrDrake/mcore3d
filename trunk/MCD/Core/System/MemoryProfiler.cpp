#include "Pch.h"
#include "MemoryProfiler.h"

#if defined(_MSC_VER)

#include "FunctionPatcher.inc"
#include "PtrVector.h"
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
		recurseCount(0),
		currentNode(nullptr), threadName(::strdup(name))
	{}

	~TlsStruct() {
		::free((void*)threadName);
	}

	AllocInfo currentAlloc;
	AllocInfo accumAlloc;
	size_t recurseCount;
	MemoryProfilerNode* currentNode;
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
	MCD_ASSUME(gTlsIndex != 0);
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

struct MyMemFooter
{
	/*! The node pointer is placed in-between the 2 fourcc values,
		ensure maximum protected as possible.
	 */
	uint32_t fourCC1;
	MCD::MemoryProfilerNode* node;
	uint32_t fourCC2;

	/*!	Magic number for HeapFree verification.
		The node pointer is not used if any of the fourcc is invalid.
		It's kind of dirty, but there is no other way to indicate a pointer
		is allocated by original HeapAlloc or our patched version.
	 */
	static const uint32_t cFourCC1 = 123456789;
	static const uint32_t cFourCC2 = 987654321;
};	// MyMemFooter

/*!	nBytes does not account for the extra footer size
	deltaBytes is non-zero when commonAlloc is used for realloc
 */
void* commonAlloc(sal_in TlsStruct* tls, sal_in void* p, size_t nBytes, int deltaBytes)
{
	size_t bytes = deltaBytes != 0 ? deltaBytes : nBytes;
	MCD_ASSUME(tls && p && "caller of commonAlloc should ensure tls and p is valid");
	MemoryProfilerNode* node = tls->currentNode;

	// If node is null, means a new thread is started
	if(!node) {
		CallstackNode* rootNode = MemoryProfiler::singleton().getRootNode();
		MCD_ASSUME(rootNode);
		tls->recurseCount++;
		tls->currentNode = node = static_cast<MCD::MemoryProfilerNode*>(
			rootNode->getChildByName(tls->threadName)
		);
		tls->recurseCount--;
	}

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);
	if(deltaBytes == 0) {
		tls->currentAlloc.count++;
		tls->accumAlloc.count++;
		node->exclusiveCount++;
		node->countSinceLastReset++;
	}
	tls->currentAlloc.bytes += bytes;
	tls->accumAlloc.bytes += bytes;
	node->exclusiveBytes += bytes;

	MyMemFooter* footer = reinterpret_cast<MyMemFooter*>(nBytes + (char*)p);
	footer->node = node;
	footer->fourCC1 = MyMemFooter::cFourCC1;
	footer->fourCC2 = MyMemFooter::cFourCC2;

	return p;
}

LPVOID WINAPI myHeapAlloc(__in HANDLE hHeap, __in DWORD dwFlags, __in SIZE_T dwBytes)
{
	TlsStruct* tls = getTlsStruct();

	// HeapAlloc will invoke itself recursivly, so we need a recursion counter
	if(!tls || tls->recurseCount > 0)
		return orgHeapAlloc(hHeap, dwFlags, dwBytes);

	tls->recurseCount++;
	void* p = orgHeapAlloc(hHeap, dwFlags, dwBytes + sizeof(MyMemFooter));
	tls->recurseCount--;

	return commonAlloc(tls, p, dwBytes, 0);
}

LPVOID WINAPI myHeapReAlloc(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem, __in SIZE_T dwBytes)
{
	TlsStruct* tls = getTlsStruct();

	if(!tls || tls->recurseCount > 0 || lpMem == nullptr || dwBytes == 0)
		return orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);

	size_t size = HeapSize(hHeap, dwFlags, lpMem);

	// On VC 2005, orgHeapReAlloc() will not invoke HeapAlloc() and HeapFree(),
	// but it does on VC 2008
	tls->recurseCount++;
	void* p = orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes + sizeof(MyMemFooter));
	tls->recurseCount--;

	int deltaSize = (dwBytes + sizeof(MyMemFooter)) - size;

	return commonAlloc(tls, p, dwBytes, deltaSize);
}

LPVOID WINAPI myHeapFree(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem)
{
	TlsStruct* tls = getTlsStruct();

	// NOTE: For VC 2008, myHeapFree may be invoked by HeapRealloc,
	// therefore we need to check recurseCount
	if(lpMem && tls && tls->recurseCount == 0)
	{
		size_t size = HeapSize(hHeap, dwFlags, lpMem) - sizeof(MyMemFooter);

		MyMemFooter* footer = (MyMemFooter*)(((char*)lpMem) + size);
		if(footer->fourCC1 == MyMemFooter::cFourCC1 && footer->fourCC2 == MyMemFooter::cFourCC2)
		{
			MCD::MemoryProfilerNode* node = reinterpret_cast<MCD::MemoryProfilerNode*>(footer->node);
			MCD_ASSUME(node);

			// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and all other
			// operations if lpMem is allocated from thread A but now free in thread B (this thread)
			ScopeRecursiveLock lock(node->mutex);
			node->exclusiveCount--;
			node->exclusiveBytes -= size;
			tls->currentAlloc.count--;
			tls->currentAlloc.bytes -= size;
		}
	}

	return orgHeapFree(hHeap, dwFlags, lpMem);
}

}	// namespace

namespace MCD {

struct MemoryProfiler::TlsList : public ptr_vector<TlsStruct>
{
	Mutex mutex;
};	// TlsList

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

void MemoryProfilerNode::reset()
{
	MemoryProfilerNode* n1, *n2;
	{	// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and myHeapFree()
		ScopeRecursiveLock lock(mutex);
		callCount = 0;
		countSinceLastReset = 0;
		n1 = static_cast<MemoryProfilerNode*>(firstChild);
		n2 = static_cast<MemoryProfilerNode*>(sibling);
	}

	if(n1) n1->reset();
	if(n2) n2->reset();
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
	mTlsList = new TlsList();
	gTlsIndex = TlsAlloc();

	setRootNode(new MemoryProfilerNode("root"));

	setEnable(enable());
}

MemoryProfiler::~MemoryProfiler()
{
	setEnable(false);

	// Delete all profiler node
	CallstackProfiler::setRootNode(nullptr);

	MCD_ASSERT(gTlsIndex != 0);
	TlsSetValue(gTlsIndex, nullptr);
	TlsFree(gTlsIndex);
	gTlsIndex = 0;

	delete mTlsList;
}

void MemoryProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
	reset();
}

void MemoryProfiler::begin(const char name[])
{
	if(!enable())
		return;

	TlsStruct* tls = getTlsStruct();
	MemoryProfilerNode* node = static_cast<MemoryProfilerNode*>(tls->currentNode);
	MCD_ASSUME(node);

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	if(name != node->name)
		node = tls->currentNode = static_cast<MemoryProfilerNode*>(node->getChildByName(name));

	node->begin();
	node->recursionCount++;
}

void MemoryProfiler::end()
{
	if(!enable())
		return;

	TlsStruct* tls = getTlsStruct();
	MemoryProfilerNode* node = static_cast<MemoryProfilerNode*>(tls->currentNode);
	MCD_ASSUME(node);

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->currentNode = static_cast<MemoryProfilerNode*>(node->parent);
}

void MemoryProfiler::nextFrame()
{
	if(!enable())
		return;

	MCD_ASSERT(getTlsStruct()->currentNode->parent == mRootNode
		&& "Do not call nextFrame() inside a profiling code block");
	++frameCount;
}

void MemoryProfiler::reset()
{
	if(!mRootNode || !enable())
		return;

	MCD_ASSERT(!getTlsStruct() || getTlsStruct()->currentNode->parent == mRootNode
		&& "Do not call reset() inside a profiling code block");
	frameCount = 0;
	static_cast<MemoryProfilerNode*>(mRootNode)->reset();
}

std::string MemoryProfiler::defaultReport(size_t nameLength) const
{
	using namespace std;
	ostringstream ss;
	AllocInfo tmpAlloc, tmpAccumAlloc;

	{	ScopeLock lock(mTlsList->mutex);
		for(TlsList::iterator i=mTlsList->begin(); i!=mTlsList->end(); ++i)
		{
			tmpAlloc.count += i->currentAlloc.count;
			tmpAlloc.bytes += i->currentAlloc.bytes;
			tmpAccumAlloc.count += i->accumAlloc.count;
			tmpAccumAlloc.bytes += i->accumAlloc.bytes;
		}
	}

	ss.flags(ios_base::left);
	ss	<< setw(30) << "Allocated count: " << setw(10) << tmpAlloc.count
		<< ", kBytes: " << float(tmpAlloc.bytes) / 1024 << endl;
	ss	<< setw(30) << "Accumulated allocated count: " << setw(10) << tmpAccumAlloc.count
		<< ", kBytes: " << float(tmpAccumAlloc.bytes) / 1024 << endl;

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
//		if(n->exclusiveCount != 0 || n->countSinceLastReset != 0)
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

void MemoryProfiler::onThreadAttach(const char* threadName)
{
	// NOTE: Allocation of TlsStruct didn't trigger commonAlloc() since we
	// haven't called TlsSetValue() yet and so myHeapAlloc will by pass it.
	TlsStruct* tls = new TlsStruct(threadName);

	{	// We haven't call TlsSetValue() yet so push_back will
		// not trigger myHeapAlloc thus no dead lock.
		ScopeLock lock(mTlsList->mutex);
		mTlsList->push_back(tls);
	}

	TlsSetValue(gTlsIndex, tls);
}

bool MemoryProfiler::enable() const
{
	return CallstackProfiler::enable;
}

void MemoryProfiler::setEnable(bool flag)
{
	CallstackProfiler::enable = flag;
	functionPatcher.UnpatchAll();

	if(flag) {
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
}

}	// namespace MCD

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
	switch(dwReason) {
	case DLL_PROCESS_ATTACH:
		// Force the profiler to instanciate
		MemoryProfiler::singleton().onThreadAttach("main thread");
		break;
	case DLL_THREAD_ATTACH:
		MemoryProfiler::singleton().onThreadAttach("worker thread");
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

void MemoryProfiler::begin(const char name[]) {}

void MemoryProfiler::end() {}

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
