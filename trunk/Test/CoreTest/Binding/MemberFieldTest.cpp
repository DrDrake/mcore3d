#include "Pch.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/ClassTraits.h"
#include "../../../MCD/Core/Binding/VMCore.h"

using namespace MCD;

namespace {

class Foo
{
public:
	Foo() : x(0), y(0), constStr("MCore!") {}
	int x, y;
	std::string str;
	const std::string constStr;
};

}	// namespace

namespace MCD {
namespace Binding {

SCRIPT_CLASS_DECLAR(Foo);
SCRIPT_CLASS_REGISTER(Foo)
	.declareClass<Foo>("Foo")
	.constructor()
	.getter("getX", &Foo::x)
	.setter("setX", &Foo::x)
	.getterSetter("getY", "setY", &Foo::y)
	.getterSetter("getStr", "setStr", &Foo::str)
	.getter("getConstStr", &Foo::constStr)
;}

}	// namespace Binding
}   // namespace MCD

TEST(MemberField_BindingTest)
{
	Binding::VMCore vm;
	Binding::ClassTraits<Foo>::bind(&vm);

	CHECK(vm.runScript("local f=Foo(); f.setX(2); assert(2==f.getX());"));
	CHECK(vm.runScript("local f=Foo(); f.setY(2); assert(2==f.getY());"));
	CHECK(vm.runScript("local f=Foo(); f.setStr(\"Hello\"); assert(\"Hello\"==f.getStr());"));
	CHECK(vm.runScript("local f=Foo(); assert(\"MCore!\"==f.getConstStr());"));
}
