#include "Pch.h"
#include "Common.h"
#include "../../MCD/Core/System/Platform.h"
#include "../../MCD/Core/System/ScriptOwnershipHandle.h"
#include "../../3Party/jkbind/Declarator.h"
#include "../../Binding/Binding.h"
#include <iostream>

using namespace MCD;

#ifndef NDEBUG
#	pragma comment(lib, "jkbindd")
#	pragma comment(lib, "squirreld")
#else
#	pragma comment(lib, "jkbind")
#	pragma comment(lib, "squirrel")
#endif

class Bar : public MCD::ScriptOwnershipHandle
{
public:
	Bar() {}

	~Bar()
	{
		std::cout << "~Bar\n";
	}

	void printMe()
	{
		std::cout << "I am bar\n";
	}
};	// Bar

class Foo : public MCD::ScriptOwnershipHandle
{
public:
	Bar* bar;

	Foo() : bar(NULL) {}

	~Foo()
	{
		std::cout << "~Foo\n";
		delete bar;
	}

	void addBar(Bar* b) {
		bar = b;
	}

	Bar* getBar() {
		return bar;
	}

	Bar* detachBar() {
		Bar* ret = bar;
		bar = NULL;
		return ret;
	}

	void printMe()
	{
		std::cout << "I am foo\n";
	}
};	// Foo

namespace script {

namespace types {

void addHandleToObject(HSQUIRRELVM v, Foo* obj, int idx)
{
	obj->setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, Foo* obj)
{
	return obj->vm && obj->pushHandle(v);
}

void addHandleToObject(HSQUIRRELVM v, Bar* obj, int idx)
{
	obj->setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, Bar* obj)
{
	return obj->vm && obj->pushHandle(v);
}

}	// namespace types


SCRIPT_CLASS_DECLAR(Foo);
SCRIPT_CLASS_DECLAR(Bar);

static void fooAddBar(Foo& self, GiveUpOwnership<Bar*> e) {
	self.addBar(e);
}

SCRIPT_CLASS_REGISTER_NAME(Foo, "Foo")
	.constructor()
	.method(L"destroy", &Foo::destroy)
	.method(L"printMe", &Foo::printMe)
	.wrappedMethod(L"addBar", &fooAddBar)
	.method<objNoCare>(L"getBar", &Foo::getBar)
	.method(L"detachBar", &Foo::detachBar)
;}

SCRIPT_CLASS_REGISTER_NAME(Bar, "Bar")
	.constructor()
	.method(L"printMe", &Bar::printMe)
;}

}	// namespace script

TEST(Basic_BindingTest)
{
	ScriptVM vm;
	script::VMCore* v = (script::VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));
	script::ClassTraits<Foo>::bind(v);
	script::ClassTraits<Bar>::bind(v);
	runScriptFile(vm, "BasicTest.nut");
	CHECK(true);
}
