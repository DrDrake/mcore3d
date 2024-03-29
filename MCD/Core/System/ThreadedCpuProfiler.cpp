#include "Pch.h"
#include "ThreadedCpuProfiler.h"
#include "Log.h"
#include "PlatformInclude.h"
#include "PtrVector.h"
#include <iomanip>
#include <sstream>

using namespace MCD;

#ifdef MCD_VC

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

	sal_notnull ThreadedCpuProfilerNode* currentNode()
	{
		// If node is null, means a new thread is started
		if(!mCurrentNode) {
			CallstackNode* rootNode = ThreadedCpuProfiler::singleton().getRootNode();
			MCD_ASSUME(rootNode);
			recurseCount++;
			mCurrentNode = static_cast<ThreadedCpuProfilerNode*>(
				rootNode->getChildByName(threadName)
			);
			recurseCount--;
		}

		return mCurrentNode;
	}

	ThreadedCpuProfilerNode* setCurrentNode(CallstackNode* node) {
		return mCurrentNode = static_cast<ThreadedCpuProfilerNode*>(node);
	}

	size_t recurseCount;
	const char* threadName;

protected:
	ThreadedCpuProfilerNode* mCurrentNode;
};	// TlsStruct

DWORD gTlsIndex = 0;

TlsStruct* getTlsStruct()
{
	MCD_ASSUME(gTlsIndex != 0);
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

}	// namespace

struct ThreadedCpuProfiler::TlsList : public ptr_vector<TlsStruct>
{
	Mutex mutex;
};	// TlsList

ThreadedCpuProfilerNode::ThreadedCpuProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent), callCount(0), inclusiveTime(uint64_t(0))
{}

void ThreadedCpuProfilerNode::begin()
{
	// Start the timer for the first call, ignore all later recursive call
	if(recursionCount == 0)
		timer.reset();

	++callCount;
}

void ThreadedCpuProfilerNode::end()
{
	if(recursionCount == 0)
		inclusiveTime += timer.get();
}

CallstackNode* ThreadedCpuProfilerNode::createNode(const char name[], CallstackNode* parent)
{
	return new ThreadedCpuProfilerNode(name, parent);
}

void ThreadedCpuProfilerNode::reset()
{
	ThreadedCpuProfilerNode* n1, *n2;
	{	// Race with ThreadedCpuProfiler::begin(), ThreadedCpuProfiler::end()
		ScopeRecursiveLock lock(mutex);
		callCount = 0;
		inclusiveTime.set(uint64_t(0));
		n1 = static_cast<ThreadedCpuProfilerNode*>(firstChild);
		n2 = static_cast<ThreadedCpuProfilerNode*>(sibling);
	}

	if(n1) n1->reset();
	if(n2) n2->reset();
}

float ThreadedCpuProfilerNode::selfTime() const
{
	// Loop and sum for all direct children
	TimeInterval sum = 0;
	const ThreadedCpuProfilerNode* n = static_cast<ThreadedCpuProfilerNode*>(firstChild);
	while(n) {
		sum += n->inclusiveTime;
		n = static_cast<ThreadedCpuProfilerNode*>(n->sibling);
	}

	return float((inclusiveTime - sum).asSecond());
}

ThreadedCpuProfiler::ThreadedCpuProfiler()
{
	mTlsList = new TlsList();
	gTlsIndex = TlsAlloc();
	setRootNode(new ThreadedCpuProfilerNode("root"));

	onThreadAttach("main thread");
}

ThreadedCpuProfiler::~ThreadedCpuProfiler()
{
	MCD_ASSERT(gTlsIndex != 0);
	TlsSetValue(gTlsIndex, nullptr);
	TlsFree(gTlsIndex);
	gTlsIndex = 0;

	// We assume that all thread will be stopped before ThreadedCpuProfiler is destroyed
	delete mTlsList;
}

void ThreadedCpuProfiler::begin(const char name[])
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	if(!tls)
		tls = reinterpret_cast<TlsStruct*>(onThreadAttach());
	ThreadedCpuProfilerNode* node = tls->currentNode();

	// Race with ThreadedCpuProfiler::reset(), ThreadedCpuProfiler::defaultReport()
	ScopeRecursiveLock lock(node->mutex);

	if(name != node->name) {
		ThreadedCpuProfilerNode* tmp = static_cast<ThreadedCpuProfilerNode*>(node->getChildByName(name));
		lock.swapMutex(tmp->mutex);
		node = tmp;

		// Only alter the current node, if the child node is not recursing
		if(node->recursionCount == 0)
			tls->setCurrentNode(node);
	}

	node->begin();
	node->recursionCount++;
}

void ThreadedCpuProfiler::end()
{
	if(!enable)
		return;

	TlsStruct* tls = getTlsStruct();
	ThreadedCpuProfilerNode* node = tls->currentNode();

	// Race with ThreadedCpuProfiler::reset(), ThreadedCpuProfiler::defaultReport()
	ScopeRecursiveLock lock(node->mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->setCurrentNode(node->parent);
}

void ThreadedCpuProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
	reset();
}

void ThreadedCpuProfiler::nextFrame()
{
	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call nextFrame() inside a profiling code block");
	++frameCount;
	timeSinceLastReset = timer.get();
}

void ThreadedCpuProfiler::reset()
{
	if(!mRootNode)
		return;

	MCD_ASSERT(mCurrentNode == mRootNode && "Do not call reset() inside a profiling code block");
	frameCount = 0;
	static_cast<ThreadedCpuProfilerNode*>(mRootNode)->reset();
	timeSinceLastReset = TimeInterval::getMax();
	timer.reset();
}

float ThreadedCpuProfiler::fps() const
{
	return float(frameCount / timeSinceLastReset.asSecond());
}

std::string ThreadedCpuProfiler::defaultReport(size_t nameLength) const
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

	for(CallstackNode* cn = mRootNode; cn; cn = CallstackNode::traverse(cn))
	{
		ThreadedCpuProfilerNode* n = static_cast<ThreadedCpuProfilerNode*>(cn);

		// Race with ThreadedCpuProfiler::begin() and ThreadedCpuProfiler::end()
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
	}

	return ss.str();
}

void* ThreadedCpuProfiler::onThreadAttach(const char* threadName)
{
	TlsStruct* tls = new TlsStruct(threadName);

	{
		ScopeLock lock(mTlsList->mutex);
		mTlsList->push_back(tls);
	}

	TlsSetValue(gTlsIndex, tls);

	return tls;
}

#include <Winsock2.h>
#pragma comment(lib, "Ws2_32")

class ThreadedCpuProfilerServer::Impl
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
			Log::write(Log::Info, "ThreadCpuProfiler client connected!!");
			return true;
		}

		return false;
	}

	void flush()
	{
		if(!connected)
			return;

		std::ostringstream ss;
		ThreadedCpuProfiler& profiler = ThreadedCpuProfiler::singleton();

		for(CallstackNode* cn = profiler.getRootNode(); cn; cn = CallstackNode::traverse(cn))
		{
			ThreadedCpuProfilerNode* n = static_cast<ThreadedCpuProfilerNode*>(cn);

			// Race with ThreadedCpuProfiler::begin() and ThreadedCpuProfiler::end()
			ScopeRecursiveLock lock(n->mutex);

			float percent = 100 * profiler.fps();
			float inclusiveTime = float(n->inclusiveTime.asSecond());

			// Skip node that have total time less than 1%
	//		if(inclusiveTime / frameCount * percent < 1)
	//			continue;

			float selfTime = n->selfTime();

			size_t callDepth = n->callDepth();
			ss	<< callDepth << ";"
				<< n << ";"
				<< n->name << ";"
				<< (inclusiveTime / profiler.frameCount * percent) << ";"
				<< (selfTime / profiler.frameCount * percent) << ";"
				<< (n->callCount == 0 ? 0 : inclusiveTime / n->callCount) << ";"
				<< (n->callCount == 0 ? 0 : selfTime / n->callCount) << ";"
				<< (float(n->callCount) / profiler.frameCount) << ";"
				<< std::endl;
		}

		std::string str = ss.str() + "\n\n";

		if(::send(clientSock, str.c_str(), int(str.length()), 0) == SOCKET_ERROR) {
			Log::format(Log::Warn, "Socket sendto() failed. At %s line %i", __FILE__, __LINE__);
			connected = false;
		}

		profiler.reset();
	}

	intptr_t sock;
	intptr_t clientSock;
	struct sockaddr_in serverAddr, clientAddr;
	bool connected;
};	// Impl

ThreadedCpuProfilerServer::ThreadedCpuProfilerServer()
	: mImpl(*new Impl)
{
	WSADATA	wsad;
	::WSAStartup(WINSOCK_VERSION, &wsad);
}

ThreadedCpuProfilerServer::~ThreadedCpuProfilerServer() {
	delete &mImpl;
	::WSACleanup();
}

bool ThreadedCpuProfilerServer::listern(uint16_t port) {
	return mImpl.listern(port);
}

bool ThreadedCpuProfilerServer::accept() {
	return mImpl.accept();
}

void ThreadedCpuProfilerServer::nextFrame() {
	ThreadedCpuProfiler::singleton().nextFrame();
}

void ThreadedCpuProfilerServer::flush() {
	mImpl.flush();
}

#else

ThreadedCpuProfiler::ThreadedCpuProfiler() {}

ThreadedCpuProfiler::~ThreadedCpuProfiler() {}

void ThreadedCpuProfiler::begin(const char name[]) {}

void ThreadedCpuProfiler::end() {}

void ThreadedCpuProfiler::nextFrame() {}

std::string ThreadedCpuProfiler::defaultReport(size_t nameLength) const {
	return "";
}

void* ThreadedCpuProfiler::onThreadAttach(const char* threadName) {
	return nullptr;
}

#endif	//_MSC_VER

ThreadedCpuProfiler& ThreadedCpuProfiler::singleton()
{
	static ThreadedCpuProfiler instance;
	return instance;
}
