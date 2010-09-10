#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"

using namespace MCD;

namespace {

class Base
{
public:
	void func1() {}
	int func2() { return 2; }
	int func3() { return 3; }
	virtual int func4() { return 4; }
};

// We will make this function appear as a member function of Base
static int wrappedFun2(Base& base) {
	return base.func2() * 100;
}

class Derived : public Base
{
public:
	int func3() { return 30; }
	sal_override int func4() { return 40; }
};

}	// namespace

namespace MCD {
namespace Binding {

SCRIPT_CLASS_DECLAR(Base);
SCRIPT_CLASS_REGISTER(Base)
	.declareClass<Base>("Base")
	.constructor()
	.method("func1", &Base::func1)
	.method("func2", &Base::func2)
	.method("func3", &Base::func3)
	.method("func4", &Base::func4)
	.wrappedMethod("func5", wrappedFun2)
;}

SCRIPT_CLASS_DECLAR(Derived);
SCRIPT_CLASS_REGISTER(Derived)
	.declareClass<Derived, Base>("Derived")
	.constructor()
	.method("baseFunc3", &Base::func3)	// Note we bind the Base's func3
	.method("func3", &Derived::func3)
;}

}	// namespace Binding
}   // namespace MCD

TEST(MemberFunction_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Base>::bind(&vm);
	Binding::ClassTraits<Derived>::bind(&vm);

//	CHECK(!vm.runScript("assert(2 == Base.func2())"));	// Calling a member function as a static one will fail
	CHECK(vm.runScript("assert(2 == Base().func2())"));
	CHECK(vm.runScript("assert(2 == Derived().func2())"));

	CHECK(vm.runScript("assert(3 == Base().func3())"));
	CHECK(vm.runScript("assert(3 == Derived().baseFunc3())"));
	CHECK(vm.runScript("assert(30 == Derived().func3())"));

	CHECK(vm.runScript("assert(4 == Base().func4())"));
	CHECK(vm.runScript("assert(40 == Derived().func4())"));

	CHECK(vm.runScript("assert(200 == Base().func5())"));
}
