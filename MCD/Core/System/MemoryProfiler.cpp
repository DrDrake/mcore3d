#include "Pch.h"
#include "MemoryProfiler.h"

#if defined(_MSC_VER)

#include "FunctionPatcher.inc"
#include "PtrVector.h"
#include <iomanip>
#include <sstream>

/*!	When working with run-time analysis tools like Intel Parallel Studio, the use of dll main
	make cause false positive, therefore we hace a macro to turn on and off the dll main.
	Use dll main has the benift of capturing more memory allocation.
 */
#define USE_DLL_MAIN 0

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
		// We want every thread have it's own copy of thread name, therefore strdup() is used
		mCurrentNode(nullptr), threadName(::strdup(name))
	{}

	~TlsStruct() {
		::free((void*)threadName);
	}

	sal_notnull MemoryProfilerNode* currentNode()
	{
		// If node is null, means a new thread is started
		if(!mCurrentNode) {
			CallstackNode* rootNode = MemoryProfiler::singleton().getRootNode();
			MCD_ASSUME(rootNode);
			recurseCount++;
			mCurrentNode = static_cast<MemoryProfilerNode*>(
				rootNode->getChildByName(threadName)
			);
			recurseCount--;
		}

		return mCurrentNode;
	}

	MemoryProfilerNode* setCurrentNode(CallstackNode* node) {
		return mCurrentNode = static_cast<MemoryProfilerNode*>(node);
	}

	AllocInfo currentAlloc;
	AllocInfo accumAlloc;
	size_t recurseCount;
	const char* threadName;

protected:
	MemoryProfilerNode* mCurrentNode;
};	// TlsStruct

typedef LPVOID (WINAPI *MyHeapAlloc)(HANDLE, DWORD, SIZE_T);
typedef LPVOID (WINAPI *MyHeapReAlloc)(HANDLE, DWORD, LPVOID, SIZE_T);
typedef LPVOID (WINAPI *MyHeapFree)(HANDLE, DWORD, LPVOID);
typedef SIZE_T (WINAPI *MyHeapSize)(HANDLE, DWORD, LPVOID);

LPVOID WINAPI myHeapAlloc(HANDLE, DWORD, SIZE_T);
LPVOID WINAPI myHeapReAlloc(HANDLE, DWORD, LPVOID, SIZE_T);
LPVOID WINAPI myHeapFree(HANDLE, DWORD, LPVOID);
SIZE_T WINAPI myHeapSize(HANDLE, DWORD, LPVOID);

FunctionPatcher functionPatcher;
MyHeapAlloc orgHeapAlloc;
MyHeapReAlloc orgHeapReAlloc;
MyHeapFree orgHeapFree;
MyHeapSize orgHeapSize;

DWORD gTlsIndex = 0;

/*!	A global mutex to protect the footer information of each allocation.
	NOTE: Intel parallel studio not able to detect the creation of a static mutex,
	therefore we need to delay it's construction until MemoryProfiler constructor.
 */
Mutex* gFooterMutex = nullptr;

TlsStruct* getTlsStruct()
{
	MCD_ASSUME(gTlsIndex != 0);
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

/*!	A footer struct that insert to every patched memory allocation,
	aim to indicate which call stack node this allocation belongs to.
 */
struct MyMemFooter
{
	/*! The node pointer is placed in-between the 2 fourcc values,
		ensure maximum protected as possible.
	 */
	uint32_t fourCC1;
	MemoryProfilerNode* node;
	uint32_t fourCC2;

	/*!	Magic number for HeapFree verification.
		The node pointer is not used if any of the fourcc is invalid.
		It's kind of dirty, but there is no other way to indicate a pointer
		is allocated by original HeapAlloc or our patched version.
	 */
	static const uint32_t cFourCC1 = 123456789;
	static const uint32_t cFourCC2 = 987654321;
};	// MyMemFooter

void commonDealloc(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem)
{
	TlsStruct* tls = getTlsStruct();

	if(lpMem && tls && tls->recurseCount == 0)
	{
//		size_t size = HeapSize(hHeap, dwFlags, lpMem);
		size_t size = orgHeapSize(hHeap, dwFlags, lpMem) - sizeof(MyMemFooter);

		ScopeLock lock1(*gFooterMutex);
		MyMemFooter* footer = (MyMemFooter*)(((char*)lpMem) + size);

		if(footer->fourCC1 == MyMemFooter::cFourCC1 && footer->fourCC2 == MyMemFooter::cFourCC2)
		{
			if(tls->recurseCount != 0)
				tls = tls;

			MemoryProfilerNode* node = reinterpret_cast<MemoryProfilerNode*>(footer->node);
			assert(node);

			// Race with MemoryProfiler::defaultReport() and all other
			// operations if lpMem is allocated from thread A but now free in thread B (this thread)
			{	ScopeUnlock unlock(gFooterMutex);	// Prevent lock hierarchy.
				ScopeRecursiveLock lock2(node->mutex);

				node->exclusiveCount--;
				node->exclusiveBytes -= size;
				tls->currentAlloc.count--;
				tls->currentAlloc.bytes -= size;
			}
		}
		else
			tls = tls;
	}
	else
		tls = tls;
}

/*!	nBytes does not account for the extra footer size
 */
void* commonAlloc(sal_in TlsStruct* tls, sal_in void* p, size_t nBytes)
{
	MCD_ASSUME(tls && p && "caller of commonAlloc should ensure tls and p is valid");

	MemoryProfilerNode* node = tls->currentNode();

	{	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
		ScopeRecursiveLock lock(node->mutex);
		tls->currentAlloc.count++;
		tls->accumAlloc.count++;
		node->exclusiveCount++;
		node->countSinceLastReset++;

		tls->currentAlloc.bytes += nBytes;
		tls->accumAlloc.bytes += nBytes;
		node->exclusiveBytes += nBytes;
	}

	{	ScopeLock lock(gFooterMutex);
		MyMemFooter* footer = reinterpret_cast<MyMemFooter*>(nBytes + (char*)p);
		footer->node = node;
		footer->fourCC1 = MyMemFooter::cFourCC1;
		footer->fourCC2 = MyMemFooter::cFourCC2;
	}

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

	return commonAlloc(tls, p, dwBytes);
}

LPVOID WINAPI myHeapReAlloc(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem, __in SIZE_T dwBytes)
{
	TlsStruct* tls = getTlsStruct();

	if(!tls || tls->recurseCount > 0)
		return orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);

	// Remove the statistics for the previous allocation first.
	commonDealloc(hHeap, dwFlags, lpMem);

	tls->recurseCount++;
	void* p = orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes + sizeof(MyMemFooter));
	tls->recurseCount--;

	return commonAlloc(tls, p, dwBytes);
}

LPVOID WINAPI myHeapFree(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem)
{
	commonDealloc(hHeap, dwFlags, lpMem);
	return orgHeapFree(hHeap, dwFlags, lpMem);
}

SIZE_T WINAPI myHeapSize(__in HANDLE hHeap, __in DWORD dwFlags, __deref LPVOID lpMem)
{
	if(dwFlags != 0)
		return orgHeapSize(hHeap, dwFlags, lpMem);
	else
		return orgHeapSize(hHeap, dwFlags, lpMem) - sizeof(MyMemFooter);
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
	exclusiveCount(0), exclusiveBytes(0), countSinceLastReset(0),
	mIsMutexOwner(false), mutex(nullptr)
{
}

MemoryProfilerNode::~MemoryProfilerNode()
{
	if(mIsMutexOwner)
		delete mutex;
}

void MemoryProfilerNode::begin()
{
	MCD_ASSERT(mutex->isLocked());
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
		ScopeRecursiveLock lock(n->mutex);
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
		ScopeRecursiveLock lock(n->mutex);
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

	// The locking of gFooterMutex should be a very short period, so use a spin lock.
	gFooterMutex = new Mutex(200);

#if !USE_DLL_MAIN
	onThreadAttach("MAIN THREAD");
#endif	// !USE_DLL_MAIN
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

	delete gFooterMutex;
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
	if(!tls)
		tls = reinterpret_cast<TlsStruct*>(onThreadAttach());
	MemoryProfilerNode* node = tls->currentNode();

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	if(name != node->name) {
		// NOTE: We have changed the node, but there is no need to lock the
		// mutex for the new node, since both mutex must be just the same instance.
		node = static_cast<MemoryProfilerNode*>(node->getChildByName(name));

		// Only alter the current node, if the child node is not recursing
		if(node->recursionCount == 0)
			tls->setCurrentNode(node);
	}

	node->begin();
	node->recursionCount++;
}

void MemoryProfiler::end()
{
	if(!enable())
		return;

	TlsStruct* tls = getTlsStruct();
	MCD_ASSERT(tls != nullptr && "MemoryProfiler::begin should already initialized TLS!");
	MemoryProfilerNode* node = tls->currentNode();

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and myHeapFree()
	ScopeRecursiveLock lock(node->mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->setCurrentNode(node->parent);
}

void MemoryProfiler::nextFrame()
{
	if(!enable())
		return;

	MCD_ASSERT(getTlsStruct()->currentNode()->parent == mRootNode
		&& "Do not call nextFrame() inside a profiling code block");
	++frameCount;
}

void MemoryProfiler::reset()
{
	if(!mRootNode || !enable())
		return;

	MCD_ASSERT(!getTlsStruct() || getTlsStruct()->currentNode()->parent == mRootNode
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
		<< setw(countWidth-2)	<< "Call/F"
		<< endl;

	MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(mRootNode);

	do
	{	// NOTE: The following std stream operation may trigger HeapAlloc,
		// there we need to use recursive mutex here.

		// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and myHeapFree()
		ScopeRecursiveLock lock(n->mutex);

		// Skip node that have no allocation at all
		if(n->callDepth() == 0 || n->exclusiveCount != 0 || n->countSinceLastReset != 0)
		{
			size_t callDepth = n->callDepth();
			const char* name = n->name;
			size_t iCount = n->inclusiveCount();
			size_t eCount = n->exclusiveCount;
			float iBytes = float(n->inclusiveBytes()) / 1024;
			float eBytes = float(n->exclusiveBytes) / 1024;
			float countSinceLastReset = float(n->countSinceLastReset) / frameCount;
			float callCount = float(n->callCount) / frameCount;

			{	// The string stream will make allocations, therefore we need to unlock the mutex
				// to prevent dead lock.
				ScopeRecursiveUnlock unlock(n->mutex);
				ss.flags(ios_base::left);
				ss	<< setw(callDepth) << ""
					<< setw(nameLength - callDepth) << name
					<< setiosflags(ios::right)// << setprecision(3)
					<< setw(countWidth)		<< iCount
					<< setw(countWidth)		<< eCount
					<< setw(bytesWidth)		<< iBytes
					<< setw(bytesWidth)		<< eBytes
					<< setw(countWidth)		<< countSinceLastReset
					<< setprecision(2)
					<< setw(countWidth-2)	<< callCount
					<< endl;
			}
		}

		n = static_cast<MemoryProfilerNode*>(CallstackNode::traverse(n));
	} while(n != nullptr);

	return ss.str();
}

void* MemoryProfiler::onThreadAttach(const char* threadName)
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

	return tls;
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
		const int prologueSize[] = { 5, 5, 5, 5 };
#else _MSC_VER > 1400	// VC 2008
		const int prologueSize[] = { 5, 5 ,5, 5 };
#endif

		// Back up the original function and then do patching
		orgHeapAlloc	= (MyHeapAlloc) functionPatcher.copyPrologue(&HeapAlloc, prologueSize[0]);
		orgHeapReAlloc	= (MyHeapReAlloc) functionPatcher.copyPrologue(&HeapReAlloc, prologueSize[1]);
		orgHeapFree		= (MyHeapFree) functionPatcher.copyPrologue(&HeapFree, prologueSize[2]);
		orgHeapSize		= (MyHeapSize) functionPatcher.copyPrologue(&HeapSize, prologueSize[3]);

		functionPatcher.patch(&HeapAlloc, &myHeapAlloc);
		functionPatcher.patch(&HeapReAlloc, &myHeapReAlloc);
		functionPatcher.patch(&HeapFree, &myHeapFree);
		functionPatcher.patch(&HeapSize, &myHeapSize);
	}
}

}	// namespace MCD

#if USE_DLL_MAIN
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
	switch(dwReason) {
	case DLL_PROCESS_ATTACH:
		// Force the profiler to instanciate
		MemoryProfiler::singleton().onThreadAttach("MAIN THREAD");
		break;
	case DLL_THREAD_ATTACH:
		MemoryProfiler::singleton().onThreadAttach();
		break;
	default:
		break;
	}
	return TRUE;
}
#endif	// USE_DLL_MAIN

#else

namespace MCD {

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent)
{}

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

void* MemoryProfiler::onThreadAttach(const char* threadName) { return nullptr; }

bool MemoryProfiler::enable() const { return false; }

void MemoryProfiler::setEnable(bool flag) { (void)flag; }

}	// namespace MCD

#endif	// _MSC_VER

CallstackNode* MemoryProfilerNode::createNode(const char name[], CallstackNode* parent)
{
	MemoryProfiler::ScopeIgnore scopeIgnore;

	MemoryProfilerNode* parentNode = static_cast<MemoryProfilerNode*>(parent);
	MemoryProfilerNode* n = new MemoryProfilerNode(name, parent);

	// Every thread should have it's own root node which owns a mutex
	if(!parentNode || parentNode->mutex == nullptr) {
		n->mutex = new RecursiveMutex();
		n->mIsMutexOwner = true;
	}
	else {
		n->mutex = parentNode->mutex;
		n->mIsMutexOwner = false;
	}

	return n;
}

MemoryProfiler& MemoryProfiler::singleton()
{
	static MemoryProfiler instance;
	return instance;
}

MemoryProfiler::ScopeIgnore::ScopeIgnore()
{
	if(TlsStruct* tls = getTlsStruct())
		tls->recurseCount++;
}

MemoryProfiler::ScopeIgnore::~ScopeIgnore()
{
	if(TlsStruct* tls = getTlsStruct())
		tls->recurseCount--;
}

#if defined(_MSC_VER)

#include <Winsock2.h>
#pragma comment(lib, "Ws2_32")

namespace MCD {

class MemoryProfilerServer::Impl
{
public:
	bool listern(uint16_t port)
	{
		if((sock = ::socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			return false;

		unsigned long nonBlocking = 1;
		if(::ioctlsocket(sock, FIONBIO, &nonBlocking) == SOCKET_ERROR)
			return false;

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = ::htons(port);
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		::memset(&(serverAddr.sin_zero), 0, 8);

		if(::bind(sock, (sockaddr*)(&serverAddr), sizeof(serverAddr)) != 0)
			return false;

		if(::listen(sock, 5) != 0)
			return false;

		return true;
	}

	bool accept()
	{
		int sin_size = sizeof(struct sockaddr_in);

		if(connected)
			return false;

		clientSock = ::accept(sock, (sockaddr*)(&clientAddr), &sin_size);

		if(clientSock != INVALID_SOCKET)
		{
			connected = true;
			std::cout << "Client connected!!" << std::endl;
			return true;
		}

		return false;
	}

	void update()
	{
		if(!connected)
			return;

		std::ostringstream ss;
		MemoryProfiler& profiler = MemoryProfiler::singleton();
		MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(profiler.getRootNode());

		do
		{	// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and myHeapFree()
			ScopeRecursiveLock lock(n->mutex);

			// Skip node that have no allocation at all
			if(n->inclusiveCount() != 0 || n->countSinceLastReset != 0)
			{
				size_t callDepth = n->callDepth();
				const char* name = n->name;
				size_t iCount = n->inclusiveCount();
				size_t eCount = n->exclusiveCount;
				float iBytes = float(n->inclusiveBytes()) / 1024;
				float eBytes = float(n->exclusiveBytes) / 1024;
				float countSinceLastReset = float(n->countSinceLastReset) / profiler.frameCount;
				float callCount = float(n->callCount) / profiler.frameCount;

				{	// The string stream will make allocations, therefore we need to unlock the mutex
					// to prevent dead lock.
					ScopeRecursiveUnlock unlock(n->mutex);
					ss	<< callDepth << ";"
						<< n << ";"	// Send the address as the node identifier
						<< name << ";"
						<< iCount << ";"
						<< eCount << ";"
						<< iBytes << ";"
						<< eBytes << ";"
						<< countSinceLastReset << ";"
						<< callCount << ";"
						<< std::endl;
				}
			}

			n = static_cast<MemoryProfilerNode*>(CallstackNode::traverse(n));
		} while(n != nullptr);

		std::string str = ss.str() + "\n\n";

		if(::send(clientSock, str.c_str(), str.length(), 0) == SOCKET_ERROR) {
			std::cout << "sendto() failed" << std::endl;
			connected = false;
		}

		profiler.reset();
	}

	int sock;
	int clientSock;
	struct sockaddr_in serverAddr, clientAddr;
	bool connected;
};	// Impl

MemoryProfilerServer::MemoryProfilerServer()
	: mImpl(new Impl())
{
	WSADATA	wsad;
	::WSAStartup(WINSOCK_VERSION, &wsad);
}

MemoryProfilerServer::~MemoryProfilerServer() {
	delete mImpl;
	::WSACleanup();
}

bool MemoryProfilerServer::listern(uint16_t port) {
	return mImpl->listern(port);
}

bool MemoryProfilerServer::accept() {
	return mImpl->accept();
}

void MemoryProfilerServer::update() {
	mImpl->update();
}

}	// namespace MCD

#endif	//_MSC_VER
