#include "Pch.h"
#include "../../../MCD/Core/Entity/Entity.h"

using namespace MCD;

namespace {

class DummyComponent1 : public IComponent {};

class DummyComponent2 : public IComponent {};

}	// namespace

TEST(Basic_ComponentTest)
{
	Entity root;

	// Note that IComponentPtr is a weak pointer
	IComponentPtr c1 = new DummyComponent1;
	IComponentPtr c2 = new DummyComponent2;

	root.addComponent(c1.get());
	root.addComponent(c2.get());

	CHECK(!root.findComponent(typeid(int)));
	CHECK_EQUAL(c1.get(), root.findComponent(typeid(DummyComponent1)));
	CHECK_EQUAL(c2.get(), root.findComponent(typeid(DummyComponent2)));

	root.removeComponent(typeid(DummyComponent1));
	CHECK(!root.findComponent(typeid(DummyComponent1)));

	// The component c1 is already deleted, so the weak pointer c1 will be null
	CHECK(!c1.get());

	// An entity can only have one instance of a certain type of IComponent,
	// so the old component will be replaced by the new one.
	root.addComponent(new DummyComponent2);
	CHECK(!c2.get());
}
