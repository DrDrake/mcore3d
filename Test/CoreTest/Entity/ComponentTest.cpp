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

// Cloning
	sal_override bool cloneable() const { return false; }
	sal_override Component* clone() const { return nullptr; }
};

class DummyComponent2 : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(DummyComponent2);
	}

// Cloning
	sal_override bool cloneable() const { return false; }
	sal_override Component* clone() const { return nullptr; }
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

// Cloning
	sal_override bool cloneable() const { return false; }
	sal_override Component* clone() const { return nullptr; }
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
	CHECK(!c1);

	// An entity can only have one instance of a certain type of Component,
	// so the old component will be replaced by the new one.
	root.addComponent(new DummyComponent21);
	CHECK(!c2);

	// Test findComponentInChildren
	std::auto_ptr<Entity> e(new Entity);
	e->name = L"Component 3";
	ComponentPtr c3 = new DummyComponent3;
	e->addComponent(c3.get());
	e->asChildOf(&root);
	e.release();

	CHECK_EQUAL(c3.get(), root.findComponentInChildren(typeid(DummyComponent3)));
}

TEST(ComponentPreorderIterator_ComponentTest)
{
	// Traversing with nothing
	for(ComponentPreorderIterator itr(nullptr); !itr.ended(); itr.next()) {
		CHECK(false);
	}

	Entity root;
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
		std::auto_ptr<Entity> e(new Entity);
		e->addComponent(c[2]);
		e->asChildOf(&root);
		e.release();

		size_t i = 0;
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(c[i], itr.current());
		}
		CHECK_EQUAL(3u, i);
	}

	{	// With an empty Entity
		Entity* e = new Entity;
		e->asChildOf(&root);

		size_t i = 0;
		for(ComponentPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(c[i], itr.current());
		}
		CHECK_EQUAL(3u, i);
	}
}
