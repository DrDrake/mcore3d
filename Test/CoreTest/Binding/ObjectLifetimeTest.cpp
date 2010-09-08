#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"

using namespace MCD;

namespace  {

static size_t baseConstructorCalled = 0;

class Base
{
public:
	Base() { ++baseConstructorCalled; }
	void func1() {}
};

class Derived : public Base
{
public:
};

class Foo
{
public:
	void func1() {}
};	// Foo

static size_t destroyCalled = 0;

}	// namespace

namespace MCD {
namespace Binding {

// Specialized destroy function for the "Base" class
template<typename T>
static void destroy(const Base* dummy, T* obj) {
	(void)dummy;
	delete obj;
	++destroyCalled;
}

SCRIPT_CLASS_DECLAR(Base);
SCRIPT_CLASS_REGISTER(Base)
	.declareClass<Base>("Base")
	.constructor()
	.method("func1", &Base::func1)
;}

SCRIPT_CLASS_DECLAR(Derived);
SCRIPT_CLASS_REGISTER(Derived)
	.declareClass<Derived, Base>("Derived")
	.constructor()
;}

SCRIPT_CLASS_DECLAR(Foo);
SCRIPT_CLASS_REGISTER(Foo)
	.declareClass<Foo>("Foo")
	.constructor()
	.method("func1", &Foo::func1)
;}

}	// namespace Binding
}   // namespace MCD

using namespace MCD;

TEST(ObjectLifeTime_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Foo>::bind(&vm);
	Binding::ClassTraits<Base>::bind(&vm);
	Binding::ClassTraits<Derived>::bind(&vm);

	baseConstructorCalled = 0;
	destroyCalled = 0;

	vm.runScript("local obj=Base();");
	CHECK_EQUAL(1u, baseConstructorCalled);
	CHECK_EQUAL(1u, destroyCalled);

	vm.runScript("local obj=Derived();");	// The class Derived will also share the same destroy() function with Base
	CHECK_EQUAL(2u, baseConstructorCalled);
	CHECK_EQUAL(2u, destroyCalled);

	// The buildin constructor would call base.construct()
	vm.runScript("class SqDerived extends Derived {} local obj=SqDerived();");
	CHECK_EQUAL(3u, baseConstructorCalled);
	CHECK_EQUAL(3u, destroyCalled);

	// Redefined constructor would NOT call base.construct()
	vm.runScript("class SqDerived extends Derived { constructor() {} } local obj=SqDerived();");
	CHECK_EQUAL(3u, baseConstructorCalled);
	CHECK_EQUAL(3u, destroyCalled);

	// We have to call base.construct() manually
	vm.runScript("class SqDerived extends Derived { constructor() { base.constructor(); } } local obj=SqDerived();");
	CHECK_EQUAL(4u, baseConstructorCalled);
	CHECK_EQUAL(4u, destroyCalled);
}
