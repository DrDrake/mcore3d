#include "Pch.h"
#include "../../../MCD/Core/Binding/CoreBindings.h"
#include "../../../MCD/Core/Binding/Declarator.h"
#include "../../../MCD/Core/Binding/VMCore.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/Entity/BehaviourComponent.h"
#include <fstream>

using namespace MCD;

namespace {

class MockComponent1 : public BehaviourComponent
{
public:
	sal_override const std::type_info& familyType() const { return typeid(*this); }
	sal_override void update(float dt) {}
};	// MockComponent1

class MockComponent2 : public BehaviourComponent
{
public:
	sal_override const std::type_info& familyType() const { return typeid(*this); }
	sal_override void update(float dt) {}
};	// MockComponent1

}	// namespace

namespace MCD {
namespace Binding {

SCRIPT_CLASS_DECLAR(MockComponent1);
SCRIPT_CLASS_REGISTER(MockComponent1)
	.declareClass<MockComponent1, Component>("MockComponent1")
	.constructor()
;}

}	// namespace Binding
}	// namespace MCD

TEST(Entity_BindingTest)
{
	Binding::VMCore vm;
	Binding::registerCoreBinding(vm);
	Binding::ClassTraits<MockComponent1>::bind(&vm);

	{	std::ifstream is("Binding/squnit.nut");
		vm.runScript(is);
	}

	{	std::ifstream is("Binding/EntityTest.nut");
		vm.runScript(is);
	}
}
