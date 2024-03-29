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

class DummyComponent21 : public DummyComponent2
{
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

	CHECK(!c1->enabled());
	CHECK(!c2->enabled());

	root.addComponent(c1);
	root.addComponent(c2);

	CHECK(c1->enabled());
	CHECK(c2->enabled());

	c1->setEnabled(false);
	c2->setEnabled(false);
	CHECK(!c1->enabled());
	CHECK(!c2->enabled());

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
	CHECK(!c1);

	// An entity can only have one instance of a certain type of Component,
	// so the old component will be replaced by the new one.
	root.addComponent(new DummyComponent21);
	CHECK(!c2);

	// Test findComponentInChildren
	Entity* e(new Entity);
	e->name = "Component 3";
	ComponentPtr c3 = new DummyComponent3;
	e->addComponent(c3);
	e->asChildOf(&root);

	CHECK_EQUAL(c3.get(), root.findComponentInChildren(typeid(DummyComponent3)));
}

namespace {

bool onAddCalled = false;
bool onRemoveCalled = false;

class TestCallbackComponent : public DummyComponent1
{
public:
	TestCallbackComponent() {
		onAddCalled = false;
		onRemoveCalled = false;
	}

	sal_override ~TestCallbackComponent() {
		if(entity())
			onRemove();
	}

	sal_override void onAdd() {
		onAddCalled = true;
	}

	sal_override void onRemove() {
		onRemoveCalled = true;
	}
};

}	// namespace

TEST(Callback_ComponentTest)
{
	Entity root;

	TestCallbackComponent* c = new TestCallbackComponent;

	CHECK(!onAddCalled);
	root.addComponent(c);
	CHECK(onAddCalled);

	CHECK(!onRemoveCalled);
	root.removeComponent(typeid(DummyComponent1));
	CHECK(onRemoveCalled);

	c = new TestCallbackComponent;
	root.addComponent(c);

	CHECK(!onRemoveCalled);
	c->destroyThis();
	CHECK(onRemoveCalled);
}

TEST(ComponentPreorderIterator_ComponentTest)
{
	// Traversing with nothing
	for(ComponentPreorderIterator itr(nullptr); !itr.ended(); itr.next()) {
		CHECK(false);
	}

	Entity root;

	{	// Start with an Entity of nothing
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next())
			CHECK(false);
	}

	Component* c[3] = { new DummyComponent1, new DummyComponent2, new DummyComponent3 };

	{	// Traversing only one Entity
		root.addComponent(c[0]);
		root.addComponent(c[1]);

		size_t i = 0;
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(c[i], itr.current());
		}
		CHECK_EQUAL(2u, i);
	}

	{	// With an child Entity
		Entity* e(new Entity);
		e->addComponent(c[2]);
		e->asChildOf(&root);

		size_t i = 0;
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(c[i], itr.current());
		}
		CHECK_EQUAL(3u, i);
	}

	{	// With an empty Entity
		Entity* e(new Entity);
		e->asChildOf(&root);

		size_t i = 0;
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(c[i], itr.current());
		}
		CHECK_EQUAL(3u, i);
	}

	{	// Traverse only part of the tree
		EntityPtr e = new Entity;
		e->asChildOf(&root);

		EntityPtr e2 = new Entity;
		e2->asChildOf(e);

		size_t i = 0;
		for(ComponentPreorderIterator itr(e.get()); !itr.ended(); itr.next(), ++i) {}
		CHECK_EQUAL(0u, i);

		e->addComponent(new DummyComponent1);
		i = 0;
		for(ComponentPreorderIterator itr(e.get()); !itr.ended(); itr.next(), ++i) {}
		CHECK_EQUAL(1u, i);
	}
}
