#include "Pch.h"
#include "../../../MCD/Core/Entity/Entity.h"
#include "../../../MCD/Core/System/Utility.h"

using namespace MCD;

namespace {

Entity* e1, *e2, *e3;
Entity* e11, *e12, *e13;
Entity* e21;

/*
	root
	|
	e3--e2--e1
	    |   |
		e21 |
		    e13--e12--e11
 */
void createTree(Entity& root)
{
	root.name = L"root";

	e1 = new Entity;
	e1->name = L"e1";
	e1->asChildOf(&root);

	e2 = new Entity;
	e2->name = L"e2";
	e2->asChildOf(&root);

	e3 = new Entity;
	e3->name = L"e3";
	e3->asChildOf(&root);

	e11 = new Entity;
	e11->name = L"e11";
	e11->asChildOf(e1);

	e12 = new Entity;
	e12->name = L"e12";
	e12->asChildOf(e1);

	e13 = new Entity;
	e13->name = L"e13";
	e13->asChildOf(e1);

	e21 = new Entity;
	e21->name = L"e21";
	e21->asChildOf(e2);
}

}	// namespace

TEST(Hierarchy_EntityTest)
{
	Entity root;
	createTree(root);

	CHECK(!root.parent());
	CHECK(!root.nextSibling());
	CHECK_EQUAL(e3, root.firstChild());

	CHECK_EQUAL(&root, e1->parent());
	CHECK_EQUAL(&root, e2->parent());
	CHECK_EQUAL(&root, e3->parent());

	// The slibings are in reverse order
	CHECK_EQUAL(e2, e3->nextSibling());
	CHECK_EQUAL(e1, e2->nextSibling());
	CHECK(!e1->nextSibling());
}

TEST(Unlink_EntityTest)
{
	{	// Unlink a leaf node
		Entity root;
		createTree(root);
		e21->unlink();

		CHECK(!e21->parent());
		CHECK(!e21->firstChild());
		CHECK(!e21->nextSibling());

		CHECK_EQUAL(&root, e2->parent());
		CHECK(!e2->firstChild());
		CHECK_EQUAL(e1, e2->nextSibling());

		delete e21;
	}

	{	// Unlink first slibing node
		Entity root;
		createTree(root);
		e3->unlink();

		CHECK(!e3->parent());
		CHECK(!e3->firstChild());
		CHECK(!e3->nextSibling());

		CHECK_EQUAL(e2, root.firstChild());

		delete e3;
	}

	{	// Unlink middle slibing node
		Entity root;
		createTree(root);
		e2->unlink();

		CHECK(!e2->parent());
		CHECK_EQUAL(e21, e2->firstChild());
		CHECK(!e2->nextSibling());

		CHECK_EQUAL(e1, e3->nextSibling());

		delete e2;
	}

	{	// Unlink last slibing node
		Entity root;
		createTree(root);
		e1->unlink();

		CHECK(!e1->parent());
		CHECK_EQUAL(e13, e1->firstChild());
		CHECK(!e1->nextSibling());

		CHECK(!e2->nextSibling());

		delete e1;
	}
}

TEST(Insertion_EntityTest)
{
	{	// Insert before a first child
		Entity root;
		createTree(root);
		e11->insertBefore(e3);
		CHECK(!e11->firstChild());
		CHECK_EQUAL(&root, e11->parent());
		CHECK_EQUAL(e3, e11->nextSibling());
		CHECK_EQUAL(e11, root.firstChild());
	}

	{	// Insert before a non-first child
		Entity root;
		createTree(root);
		e11->insertBefore(e2);
		CHECK(!e11->firstChild());
		CHECK_EQUAL(&root, e11->parent());
		CHECK_EQUAL(e2, e11->nextSibling());
	}

	{	// Insert after
		Entity root;
		createTree(root);
		e11->insertAfter(e2);
		CHECK(!e11->firstChild());
		CHECK_EQUAL(&root, e11->parent());
		CHECK_EQUAL(e1, e11->nextSibling());
	}
}

TEST(WorldTransform_EntityTest)
{
	Entity root;
	createTree(root);

	root.localTransform.setTranslation(Vec3f(1, 2, 3));
	e1->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver2(), 0));
	e13->localTransform.setTranslation(Vec3f(3, 2, 1));

	Vec3f v(0.0f);
	e13->worldTransform().transformPoint(v);

	CHECK(v.isNearEqual(Vec3f(2, 4, 0)));

	// Test setWorldTransform
	e13->setWorldTransform(Mat44f::cIdentity);
	CHECK(e13->worldTransform().isNearEqual(Mat44f::cIdentity));
}

TEST(PreorderIterator_EntityTest)
{
	// Traversing with nothing
	for(EntityPreorderIterator itr(nullptr); !itr.ended(); itr.next()) {
		CHECK(false);
	}

	Entity root;

	{	// Traversing only one node
		size_t i = 0;
		for(EntityPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {}
		CHECK_EQUAL(1u, i);
	}

	createTree(root);

	{	// Traversing a more complicated tree
		size_t i = 0;
		Entity* expected[] = { &root, e3, e2, e21, e1, e13, e12, e11 };

		for(EntityPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(expected[i], itr.operator->());
		}
		CHECK_EQUAL(MCD_COUNTOF(expected), i);
	}

	{	// Traversing a sub-tree only
		size_t i = 0;
		Entity* expected[] = { e1, e13, e12, e11 };

		for(EntityPreorderIterator itr(e1); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(expected[i], itr.operator->());
		}
		CHECK_EQUAL(MCD_COUNTOF(expected), i);
	}

	{	// Test skipChildren()
		EntityPreorderIterator itr(nullptr);
		CHECK(!itr.skipChildren());
	}
}

TEST(Find_EntityTest)
{
	Entity root;
	CHECK(!root.findEntityInChildren(L"e21"));

	createTree(root);
	CHECK_EQUAL(e21, root.findEntityInChildren(L"e21"));
}

namespace Clone_EntityTest
{
	class CloneableComponent : public Component
	{
	public:
		std::string data;

		CloneableComponent(const std::string& _data) : data(_data) {}
		CloneableComponent(const CloneableComponent& c) : data(c.data) {}

		sal_override const std::type_info& familyType() const { return typeid(CloneableComponent); }
		sal_override bool cloneable() const { return true; }
		sal_override Component* clone() const { return new CloneableComponent(*this); }
	};	// CloneableComponent

	class NonCloneableComponent : public Component
	{
	public:
		std::string data;

		NonCloneableComponent(const std::string& _data) : data(_data) {}

		sal_override const std::type_info& familyType() const { return typeid(NonCloneableComponent); }
		sal_override bool cloneable() const { return false; }
		sal_override Component* clone() const { return nullptr; }
	};	// NonCloneableComponent
}	// namespace Clone_EntityTest

TEST(Clone_EntityTest)
{
	using namespace Clone_EntityTest;

	Entity root;
	createTree(root);

	// Change some attributes
	e2->enabled = false;
	e3->enabled = false;

	// Add some components
	e1->addComponent(new CloneableComponent("c1"));
	e1->addComponent(new NonCloneableComponent("nc1"));
	e2->addComponent(new CloneableComponent("c2"));
	e3->addComponent(new CloneableComponent("c3"));
	e11->addComponent(new CloneableComponent("c11"));
	e12->addComponent(new CloneableComponent("c12"));
	e13->addComponent(new CloneableComponent("c13"));

	// Apply some transformations
	root.localTransform.setTranslation(Vec3f(1, 2, 3));
	e1->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver2(), 0));
	e13->localTransform.setTranslation(Vec3f(3, 2, 1));

	std::auto_ptr<Entity> clone_root(root.clone());
	Entity* clone_e1 = clone_root->findEntityInChildren(L"e1");
	Entity* clone_e2 = clone_root->findEntityInChildren(L"e2");
	Entity* clone_e3 = clone_root->findEntityInChildren(L"e3");
	Entity* clone_e11 = clone_root->findEntityInChildren(L"e11");
	Entity* clone_e12 = clone_root->findEntityInChildren(L"e12");
	Entity* clone_e13 = clone_root->findEntityInChildren(L"e13");

	{
		bool allNotNull = clone_e1 && clone_e2 && clone_e3 && clone_e11 && clone_e12 && clone_e13;
		CHECK(allNotNull);
		if(!allNotNull)
			return;
	}

	CHECK_EQUAL(clone_root->name, L"root");
	CHECK_EQUAL(clone_e1->name, L"e1");
	CHECK_EQUAL(clone_e2->name, L"e2");
	CHECK_EQUAL(clone_e3->name, L"e3");
	CHECK_EQUAL(clone_e11->name, L"e11");
	CHECK_EQUAL(clone_e12->name, L"e12");
	CHECK_EQUAL(clone_e13->name, L"e13");

	// Check the clone_root's structure
	CHECK(!clone_root->parent());
	CHECK(!clone_root->nextSibling());
	CHECK_EQUAL(clone_e3, clone_root->firstChild());

	CHECK_EQUAL(clone_root.get(), clone_e1->parent());
	CHECK_EQUAL(clone_root.get(), clone_e2->parent());
	CHECK_EQUAL(clone_root.get(), clone_e3->parent());

	// The slibings are in reverse order
	CHECK_EQUAL(clone_e2, clone_e3->nextSibling());
	CHECK_EQUAL(clone_e1, clone_e2->nextSibling());
	CHECK(!clone_e1->nextSibling());

	// Verift the cloned attributes
	CHECK_EQUAL(true, clone_e1->enabled);
	CHECK_EQUAL(false, clone_e2->enabled);
	CHECK_EQUAL(false, clone_e3->enabled);

	// Verify the cloned transformations
	Vec3f v(0.0f);
	clone_e13->worldTransform().transformPoint(v);

	CHECK(v.isNearEqual(Vec3f(2, 4, 0)));

	// Verify the cloned Components
	CHECK(clone_e1->findComponent<NonCloneableComponent>() == nullptr);

#ifdef MCD_VC
#	pragma warning(push)
#	pragma warning(disable: 6011)
#endif
	CHECK_EQUAL("c1", clone_e1->findComponent<CloneableComponent>()->data);
	CHECK_EQUAL("c2", clone_e2->findComponent<CloneableComponent>()->data);
	CHECK_EQUAL("c3", clone_e3->findComponent<CloneableComponent>()->data);
	CHECK_EQUAL("c11", clone_e11->findComponent<CloneableComponent>()->data);
	CHECK_EQUAL("c12", clone_e12->findComponent<CloneableComponent>()->data);
	CHECK_EQUAL("c13", clone_e13->findComponent<CloneableComponent>()->data);
#ifdef MCD_VC
#	pragma warning(pop)
#endif
}
