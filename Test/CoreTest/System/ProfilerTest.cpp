#include "Pch.h"
#include "../../../MCD/Core/System/CpuProfiler.h"

using namespace MCD;

namespace {

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

void funA();
void funB();
void funC();
void funD();

// Recurse itself
void recurse(int count);

/*!	Recurse between two functions:
	recurse1 -> recurse2 -> recurse1 -> recurse2 ...
 */
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
void funA() {
	ScopeProfiler profile("funA");
	funB();
	funD();
}

void funB() {
	ScopeProfiler profile("funB");
	funC();
	recurse1(10);
}

void funC() {
	ScopeProfiler profile("funC");
	recurse(10);
	funD();
}

void funD() {
	ScopeProfiler profile("funD");
}

void recurse(int count) {
	ScopeProfiler profile("recurse");
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
}

}	// namespace CpuProfilerTest

}	// namespace

TEST(ProfilerTest)
{
	using namespace CpuProfilerTest;

	profiler.setRootNode(new CpuProfilerNode("root"));
	profiler.enable = true;
	profiler.reset();

	for(int i=0; i<10; ++i) {
		funA();
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
void funC(void* newByFunB);
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

	b = realloc(nullptr, 10);
	b = realloc(b, 20);		// Most likely the memory pointer does not altered
	b = realloc(b, 2000);	// Most likely the memory pointer is altered
	realloc(b, 0);

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

void funC(void* newByFunB)
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
}

}	// namespace MemoryProfilerTest

}	// namespace

TEST(MemoryProfilerTest)
{
	using namespace MemoryProfilerTest;

	MemoryProfiler::singleton().setRootNode(new MemoryProfilerNode("root"));
	MemoryProfiler::singleton().enable = true;
	MemoryProfiler::singleton().reset();

	for(int i=0; i<10; ++i) {
		funA();
		MemoryProfiler::singleton().nextFrame();
	}

	std::string s = MemoryProfiler::singleton().defaultReport(20);
	CHECK(!s.empty());
	std::cout << s << std::endl;
}
