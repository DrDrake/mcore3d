#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"

using namespace MCD;

namespace {

class Base
{
public:
	static Base* singleton() { static Base obj; return &obj; }
};

class Derived : public Base
{
public:
	static Derived* singleton() { static Derived obj; return &obj; }
};

class Foo
{
public:
	static void func1() {}
	static int func2() { return 123; }
	static int func3(int x) { return x; }
	static float func4(float v1, float v2) { return v1 + v2; }
	static bool func5(Base* p1, Derived* p2) { return p1 == Base::singleton() && p2 == Derived::singleton(); }
	static bool func6(Base* p1, Derived* p2) { return p1 == p2 && p2 == Derived::singleton(); }
	static bool func7(Base* p1) { return p1 == nullptr; }				// Test for passing null as object pointer
	static bool func8(Base& p1) { return &p1 == Base::singleton(); }	// Test for passing object reference
	static bool func9(Base p1) { return &p1 != Base::singleton(); }		// Test for passing object value
};	// Foo

}	// namespace

namespace MCD {
namespace Binding {

SCRIPT_CLASS_DECLAR(Base);
SCRIPT_CLASS_REGISTER(Base)
	.declareClass<Base>("Base")
	.staticMethod<objNoCare>("singleton", &Base::singleton)
;}

SCRIPT_CLASS_DECLAR(Derived);
SCRIPT_CLASS_REGISTER(Derived)
	.declareClass<Derived, Base>("Derived")
	.staticMethod<objNoCare>("singleton", &Derived::singleton)
;}

static int rawFuncAdd2Float(HSQUIRRELVM vm)
{
	const int paramCount = sq_gettop(vm) - 1;
	if(paramCount != 2)
		return sq_throwerror(vm, "Expecting 2 parameter");

	if(!match(TypeSelect<float>(), vm, -1) || !match(TypeSelect<float>(), vm, -2))
		return sq_throwerror(vm, "Expecting 2 numeric parameters");

	float v1 = get(TypeSelect<float>(), vm, -1);
	float v2 = get(TypeSelect<float>(), vm, -2);

	push(vm, v1 + v2);

	return 1;	// One value returned
}

SCRIPT_CLASS_DECLAR(Foo);
SCRIPT_CLASS_REGISTER(Foo)
	.declareClass<Foo>("Foo")
	.staticMethod("func1", &Foo::func1)
	.staticMethod("func2", &Foo::func2)
	.staticMethod("func3", &Foo::func3)
	.staticMethod("func4", &Foo::func4)
	.staticMethod("func5", &Foo::func5)
	.staticMethod("func6", &Foo::func6)
	.staticMethod("func7", &Foo::func7)
	.staticMethod("func8", &Foo::func8)
//	.staticMethod("func9", &Foo::func9)	// Taking value of object is not supported yet
	.staticMethod("func10", &rawFuncAdd2Float)
;}

}	// namespace Binding
}   // namespace MCD

TEST(StaticFunction_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Foo>::bind(&vm);
	Binding::ClassTraits<Base>::bind(&vm);
	Binding::ClassTraits<Derived>::bind(&vm);

	CHECK(vm.runScript("Foo.func1()"));
	CHECK(vm.runScript("assert(123 == Foo.func2())"));
	CHECK(vm.runScript("assert(123 == Foo.func3(123))"));
	CHECK(vm.runScript("assert(3 == Foo.func4(1, 2))"));
	CHECK(vm.runScript("assert(Foo.func5(Base.singleton(), Derived.singleton()))"));
	CHECK(vm.runScript("assert(Foo.func6(Derived.singleton(), Derived.singleton()))"));	// Passing derived object to a C function which expecting a base pointer

	CHECK(vm.runScript("assert(Foo.func7(null))"));
	CHECK(vm.runScript("assert(Foo.func8(Base.singleton()))"));
//	CHECK(!vm.runScript("assert(Foo.func8(null))"));	// Passing null to a C function which expecting an object reference will fail.
	CHECK(vm.runScript("assert(444 == Foo.func10(123, 321))"));
}
