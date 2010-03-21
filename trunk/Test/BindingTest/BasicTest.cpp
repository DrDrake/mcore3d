#include "Pch.h"
#include "Common.h"
#include "../../MCD/Binding/Binding.h"
#include "../../MCD/Core/System/Platform.h"
#include "../../MCD/Core/System/ScriptOwnershipHandle.h"
#include "../../3Party/jkbind/Declarator.h"
#include <iostream>

using namespace MCD;

#ifdef MCD_VC
#	ifndef NDEBUG
#		pragma comment(lib, "jkbindd")
#		pragma comment(lib, "squirreld")
#	else
#		pragma comment(lib, "jkbind")
#		pragma comment(lib, "squirrel")
#	endif
#endif

class Bar : public MCD::ScriptOwnershipHandle
{
public:
	Bar() {}

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

	~Foo() {
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

	void acceptNullBar(Bar* b) {
		MCD_ASSERT(b == nullptr);
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
	.method("destroy", &Foo::destroy)
	.method("printMe", &Foo::printMe)
	.wrappedMethod("addBar", &fooAddBar)
	.method<objNoCare>("getBar", &Foo::getBar)
	.method("detachBar", &Foo::detachBar)
	.method("acceptNullBar", &Foo::acceptNullBar)
;}

SCRIPT_CLASS_REGISTER_NAME(Bar, "Bar")
	.constructor()
	.method("printMe", &Bar::printMe)
;}

}	// namespace script

TEST(Basic_BindingTest)
{
	ScriptVM vm;
	script::VMCore* v = (script::VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));
	script::ClassTraits<Foo>::bind(v);
	script::ClassTraits<Bar>::bind(v);
	CHECK(runScriptFile(vm, "BasicTest.nut"));
}

TEST(ScriptInheritCppClass_BindingTest)
{
	ScriptVM vm;
	script::VMCore* v = (script::VMCore*)sq_getforeignptr(HSQUIRRELVM(vm.getImplementationHandle()));
	script::ClassTraits<Foo>::bind(v);
	script::ClassTraits<Bar>::bind(v);
	CHECK(runScriptFile(vm, "ScriptInheritCppClassTest.nut"));
}

TEST(Coroutine_BindingTest)
{
	ScriptVM vm;
	CHECK(runScriptFile(vm, "CoroutineTest.nut"));
}
