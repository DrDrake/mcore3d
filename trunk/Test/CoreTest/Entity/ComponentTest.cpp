#include "Pch.h"
#include "../../../MCD/Core/Entity/Component.h"
#include "../../../MCD/Core/Entity/Entity.h"

using namespace MCD;

namespace {

class DummyComponent1 : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(DummyComponent1);
	}
};

class DummyComponent2 : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(DummyComponent2);
	}
};

class DummyComponent3 : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(DummyComponent3);
	}
};

}	// namespace

TEST(Basic_ComponentTest)
{
	Entity root;

	// Note that ComponentPtr is a weak pointer
	ComponentPtr c1 = new DummyComponent1;
	ComponentPtr c2 = new DummyComponent2;

	root.addComponent(c1.get());
	root.addComponent(c2.get());

	CHECK_EQUAL(&root, c1->entity());
	CHECK_EQUAL(&root, c2->entity());

	CHECK(typeid(DummyComponent1) == c1->familyType());
	CHECK(typeid(DummyComponent2) == c2->familyType());

	CHECK(!root.findComponent(typeid(int)));
	CHECK_EQUAL(c1.get(), root.findComponent(typeid(DummyComponent1)));
	CHECK_EQUAL(c2.get(), root.findComponent(typeid(DummyComponent2)));

	root.removeComponent(typeid(DummyComponent1));
	CHECK(!root.findComponent(typeid(DummyComponent1)));

	// The component c1 is already deleted, so the weak pointer c1 will be null
	CHECK(!c1.get());

	// An entity can only have one instance of a certain type of Component,
	// so the old component will be replaced by the new one.
	root.addComponent(new DummyComponent2);
	CHECK(!c2.get());

	// Test findComponentInChildren
	Entity* e = new Entity;
	e->name = L"Component 3";
	e->link(&root);
	ComponentPtr c3 = new DummyComponent3;
	e->addComponent(c3.get());

	CHECK_EQUAL(c3.get(), root.findComponentInChildren(typeid(DummyComponent3)));
}
