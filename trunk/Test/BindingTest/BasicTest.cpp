#include "Pch.h"
#include "Common.h"
#include "../../MCD/Core/System/Platform.h"
#include "../../3Party/jkbind/Declarator.h"
#include "../../Binding/Binding.h"
#include "../../3Party/squirrel/squirrel.h"
#include "../../3Party/squirrel/squirrel/sqclass.h"
#include "../../3Party/squirrel/squirrel/sqclass.h"
#include <iostream>

using namespace MCD;

#ifndef NDEBUG
#	pragma comment(lib, "jkbindd")
#	pragma comment(lib, "squirreld")
#else
#	pragma comment(lib, "jkbind")
#	pragma comment(lib, "squirrel")
#endif

namespace script {

class CppOwnershipHandle
{
public:
	CppOwnershipHandle() : vm(nullptr) {}
	~CppOwnershipHandle() { destroy(); }

	void destroy()
	{
		if(!vm)
			return;

		// Get the SQObject of what the weak reference pointing to.
		SQObject& o = weakRef._unVal.pWeakRef->_obj;

		// The SQObject may be null, if there are no more reference to this object
		// in the VM. That's also the reason why we need to store SQObject via weak
		// reference but not the SQObject itself.
		if(sq_isinstance(o)) {
			// Destroy (in squirrel api it's call Finalize) the corresponding handle
			// of this object.
			o._unVal.pInstance->Finalize();
		}

		// Release the WEAK reference
		sq_release(vm, &weakRef);

		vm = nullptr;
	}

	/*!	Associate a weak pointer pointing to the object at index.
		If the supplied HSQUIRRELVM is null, then any previous
		associated handle will be removed.
	 */
	void setHandle(HSQUIRRELVM v, int index)
	{
		if(vm)	// Remove previous reference
			sq_release(vm, &weakRef);
		vm = v;

		if(!v)
			return;

		sq_weakref(vm, index);	// Create a weak reference to the object at index
		jkSCRIPT_API_VERIFY(sq_getstackobj(vm, index, &weakRef));
		sq_addref(vm, &weakRef);
		sq_pop(vm, 1);
	}

	HSQUIRRELVM vm;
	HSQOBJECT weakRef;
};	// CppOwnershipHandle

}	// namespace script

class Bar : public script::CppOwnershipHandle
{
public:
	Bar() {}

	MCD_NOINLINE ~Bar()
	{
		std::cout << "~Bar\n";
	}

	void printMe()
	{
		std::cout << "I am bar\n";
	}
};	// Bar

class Foo : public script::CppOwnershipHandle
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

void pushHandle(Bar* obj, HSQUIRRELVM v, HSQOBJECT handle)
{
//	obj->vm = v;
//	obj->object = handle;
}

SCRIPT_CLASS_DECLAR(Foo);
SCRIPT_CLASS_DECLAR(Bar);

static int fooCreate(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	Foo* foo = new Foo;
	construct::pushResult(vm, foo);
	foo->setHandle(vm, -1);
	return 1;
}

static void fooAddBar(Foo& self, GiveUpOwnership<Bar*> e) {
	self.addBar(e);
}

SCRIPT_CLASS_REGISTER_NAME(Foo, "Foo")
	.rawMethod(L"constructor", fooCreate)
	.method(L"destroy", &Foo::destroy)
	.method(L"printMe", &Foo::printMe)
	.wrappedMethod(L"addBar", &fooAddBar)
	.method<objNoCare>(L"getBar", &Foo::getBar)
	.method(L"detachBar", &Foo::detachBar)
;}

static int barCreate(HSQUIRRELVM vm)
{
	script::detail::StackHandler sa(vm);
	Bar* bar = new Bar;
	construct::pushResult(vm, bar);
	bar->setHandle(vm, -1);
	return 1;
}

SCRIPT_CLASS_REGISTER_NAME(Bar, "Bar")
	.rawMethod(L"constructor", barCreate)
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
