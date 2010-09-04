#include "Pch.h"
#include "../../../MCD/Core/System/CpuProfiler.h"

#ifdef MCD_VC

using namespace MCD;

namespace {

/*
	A
	|--B
	|  |--C
	|  |  |--recurse(10)
	|  |  |--D
	|  |
	|  |--recurse1(10)
	|     |--recurse2(10)
	|--D
 */
template<class S>
class FunctionTester
{
public:
	void funA() {
		S profile("funA");
		funB();
		funD();
	}

	void funB() {
		S profile("funB");
		funC();
		recurse1(10);
	}

	void funC() {
		S profile("funC");
		recurse(10);
		funD();
	}

	void funD() {
		S profile("funD");
	}

	// Recurse itself
	void recurse(int count) {
		S profile("recurse");
		if(count > 0)
			recurse(count - 1);
	}

	/*!	Recurse between two functions:
		recurse1 -> recurse2 -> recurse1 -> recurse2 ...
	 */
	void recurse1(int count) {
		S profile("recurse1");
		recurse2(count);
	}

	void recurse2(int count) {
		S profile("recurse2");
		if(count > 0)
			recurse1(count - 1);
	}
};	// FunctionTester

namespace CpuProfilerTest {

CpuProfiler profiler;

class ScopeProfiler : Noncopyable
{
public:
	ScopeProfiler(const char name[]) {
		profiler.begin(name);
	}

	~ScopeProfiler() {
		profiler.end();
	}

private:
	ScopeProfiler(const ScopeProfiler& rhs);
	ScopeProfiler& operator=(const ScopeProfiler& rhs);
};	// ScopeProfiler

}	// namespace CpuProfilerTest

}	// namespace

TEST(CpuProfilerTest)
{
	using namespace CpuProfilerTest;

	profiler.setRootNode(new CpuProfilerNode("root"));
	profiler.enable = true;
	profiler.reset();

	FunctionTester<ScopeProfiler> tester;

	for(int i=0; i<10; ++i) {
		tester.funA();
		profiler.nextFrame();
	}

	std::string s = profiler.defaultReport(20);
	CHECK(!s.empty());
	std::cout << s << std::endl;

	CHECK(profiler.getRootNode()->firstChild);
	CHECK(!profiler.getRootNode()->sibling);

	CallstackNode* a = profiler.getRootNode()->firstChild;
	CHECK(a->firstChild);
	CHECK(!a->sibling);
	CHECK_EQUAL(std::string("funA"), a->name);

	CallstackNode* b = a->firstChild;
	CHECK(b->firstChild);
	CHECK(b->sibling);
	CHECK_EQUAL(std::string("funB"), b->name);

	CallstackNode* d = b->sibling;
	CHECK(!d->firstChild);
	CHECK(!d->sibling);
	CHECK_EQUAL(std::string("funD"), d->name);

	CallstackNode* c = b->firstChild;
	CHECK(c->firstChild);
	CHECK(c->sibling);
	CHECK_EQUAL(std::string("funC"), c->name);

	CallstackNode* r = c->firstChild;
	CHECK(!r->firstChild);
	CHECK(r->sibling);
	CHECK_EQUAL(std::string("recurse"), r->name);

	d = r->sibling;
	CHECK(!d->firstChild);
	CHECK(!d->sibling);
	CHECK_EQUAL(std::string("funD"), d->name);

	CallstackNode* r1 = c->sibling;
	CHECK(r1->firstChild);
	CHECK(!r1->sibling);
	CHECK_EQUAL(std::string("recurse1"), r1->name);

	CallstackNode* r2 = r1->firstChild;
	CHECK(!r2->firstChild);
	CHECK(!r2->sibling);
	CHECK_EQUAL(std::string("recurse2"), r2->name);
}

#include "../../../MCD/Core/System/Thread.h"
#include "../../../MCD/Core/System/ThreadedCpuProfiler.h"

namespace {

namespace ThreadedCpuProfilerTest {

class ScopeProfiler : Noncopyable
{
public:
	ScopeProfiler(const char name[]) {
		ThreadedCpuProfiler::singleton().begin(name);
	}

	~ScopeProfiler() {
		ThreadedCpuProfiler::singleton().end();
	}

private:
	ScopeProfiler(const ScopeProfiler& rhs);
	ScopeProfiler& operator=(const ScopeProfiler& rhs);
};	// ScopeProfiler

//! Keep active until the thread inform it to quit
class LoopRunnable : public MCD::Thread::IRunnable
{
public:
	typedef FunctionTester<ScopeProfiler> Tester;

	LoopRunnable(Tester& tester) : mTester(tester), mLoopCount(0) {}

protected:
	sal_override void run(Thread& thread)
	{
		ThreadedCpuProfiler::singleton().onThreadAttach("my worker thread");

		while(thread.keepRun()) {
			ScopeProfiler profile("LoopRunnable::run");
			mTester.funA();
			++mLoopCount;
		}
	}

private:
	Tester& mTester;
	size_t mLoopCount;
};	// LoopRunnable

}	// namespace ThreadedCpuProfilerTest

}	// namespace

TEST(ThreadedCpuProfilerTest)
{
	using namespace ThreadedCpuProfilerTest;

	ThreadedCpuProfiler& profiler = ThreadedCpuProfiler::singleton();
	profiler.enable = true;
//	profiler.reset();
	FunctionTester<ScopeProfiler> tester;

	LoopRunnable runnable1(tester), runnable2(tester);
	Thread thread1(runnable1, false);
	Thread thread2(runnable2, false);

	// Give up the CPU time for the other 2 thread to run
	mSleep(0);

	for(int i=0; i<1000; ++i) {
		tester.funA();
		profiler.nextFrame();
	}

	thread1.wait();
	thread2.wait();

	std::string s = profiler.defaultReport(20);
	CHECK(!s.empty());
	std::cout << s << std::endl;
}

#include "../../../MCD/Core/System/MemoryProfiler.h"

namespace {

namespace MemoryProfilerTest {

class ScopeProfiler : Noncopyable
{
public:
	ScopeProfiler(const char name[]) {
		MemoryProfiler::singleton().begin(name);
	}

	~ScopeProfiler() {
		MemoryProfiler::singleton().end();
	}
};	// ScopeProfiler

void funA();
void funB();
void funC(int* newByFunB);
void funD();

// Recurse itself
void recurse(int count);

void recurse1(int count);
void recurse2(int count);

/*
	A
	|--B
	|  |--C
	|  |  |--recurse(10)
	|  |  |--D
	|  |
	|  |--recurse1(10)
	|     |--recurse2(10)
	|--D
 */
void funA()
{
	ScopeProfiler profile("funA");
	void* b = malloc(10);
	free(b);

	b = malloc(0);
	free(b);
	free(nullptr);

#ifdef MCD_VC
#	pragma warning (push)
#	pragma warning (disable : 6308)
#endif
	b = realloc(nullptr, 10);
	b = realloc(b, 20);		// Most likely the memory pointer does not altered
	b = realloc(b, 2000);	// Most likely the memory pointer is altered
	b = realloc(b, 0);

#ifndef MCD_APPLE
	// XCode gives non null value, which I think it's not a standard behaviour
	assert(b == nullptr);
#endif

#ifdef MCD_VC
#	pragma warning (pop)
#endif

	b = calloc(10, 4);
	free(b);
	std::string s("hello abcdefa asdfa sdfa sdfasgfadgadfg asdfgadfg afdsg!");
//	printf("%i, %i, %s", 1, 2, s.c_str());
	funB();
	funD();
}

void funB() {
	ScopeProfiler profile("funB");
	funC(new int[10]);
	recurse1(10);
}

void funC(int* newByFunB)
{
	ScopeProfiler profile("funC");
	delete[] newByFunB;
	recurse(10);
	funD();
}

void funD()
{
	ScopeProfiler profile("funD");
}

void recurse(int count) {
	ScopeProfiler profile("recurse");
	free(malloc(1));
	if(count > 0)
		recurse(count - 1);
}

void recurse1(int count) {
	ScopeProfiler profile("recurse1");
	recurse2(count);
}

void recurse2(int count) {
	ScopeProfiler profile("recurse2");
	if(count > 0)
		recurse1(count - 1);
	void* b = malloc(1);
	free(b);
}

}	// namespace MemoryProfilerTest

}	// namespace

TEST(MemoryProfilerTest)
{
	using namespace MemoryProfilerTest;

	MemoryProfiler::singleton().setEnable(true);

	for(int i=0; i<10; ++i) {
		funA();
		MemoryProfiler::singleton().nextFrame();
	}

	std::string s = MemoryProfiler::singleton().defaultReport(20);
	CHECK(!s.empty());
	std::cout << s << std::endl;
}

namespace {

//! Keep active until the thread inform it to quit
class LoopRunnable : public MCD::Thread::IRunnable
{
public:
	LoopRunnable() : LoopCount(0) {}

protected:
	sal_override void run(Thread& thread)
	{
		using namespace MemoryProfilerTest;
		while(thread.keepRun()) {
			ScopeProfiler profile("LoopRunnable::run");
			funA();
			++LoopCount;
		}
	}

private:
	size_t LoopCount;
};	// LoopRunnable

}	// namespace

TEST(MemoryProfilerWithThreadTest)
{
	using namespace MemoryProfilerTest;

	MemoryProfiler::singleton().setEnable(true);

	LoopRunnable runnable1, runnable2;

	{	Thread dummy(runnable1, false);
		dummy.wait();
	}

	Thread thread1(runnable1, false);
	Thread thread2(runnable2, false);

	// Give up the CPU time for the other 2 thread to run
	mSleep(0);

	for(int i=0; i<1000; ++i) {
		funA();
		MemoryProfiler::singleton().nextFrame();
		if(i % 5 == 0)
			MemoryProfiler::singleton().reset();
	}

	thread1.wait();
	thread2.wait();

	std::string s = MemoryProfiler::singleton().defaultReport(20);
	CHECK(!s.empty());
	std::cout << s << std::endl;
}

#endif	// MCD_VC
