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
	root.name = "root";

	e1 = new Entity;
	e1->name = "e1";
	e1->asChildOf(&root);

	e2 = new Entity;
	e2->name = "e2";
	e2->asChildOf(&root);

	e3 = new Entity;
	e3->name = "e3";
	e3->asChildOf(&root);

	e11 = new Entity;
	e11->name = "e11";
	e11->asChildOf(e1);

	e12 = new Entity;
	e12->name = "e12";
	e12->asChildOf(e1);

	e13 = new Entity;
	e13->name = "e13";
	e13->asChildOf(e1);

	e21 = new Entity;
	e21->name = "e21";
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

	CHECK(!root.isAncestorOf(root));
	CHECK_EQUAL(1u, root.isAncestorOf(*e1));
	CHECK_EQUAL(1u, root.isAncestorOf(*e2));
	CHECK_EQUAL(2u, root.isAncestorOf(*e11));
	CHECK(!e1->isAncestorOf(root));
	CHECK(!e1->isAncestorOf(*e2));
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
		const Entity* expected[] = { &root, e3, e2, e21, e1, e13, e12, e11 };
		int expectedDepthChange[] = { 0, 1, 0, 1, -1, 1, 0, 0 };

		for(EntityPreorderIterator itr(&root); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(expected[i], itr.operator->());
			CHECK_EQUAL(expectedDepthChange[i], itr.depthChange());
		}
		CHECK_EQUAL(MCD_COUNTOF(expected), i);
	}

	{	// Travers a leaf node in a tree
		EntityPreorderIterator itr(e21);
		CHECK_EQUAL(e21, itr.current());
		itr.next();
		CHECK_EQUAL((Entity*)nullptr, itr.current());
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
	CHECK(!root.findEntityInDescendants("e21"));

	createTree(root);
	CHECK_EQUAL(e21, root.findEntityInDescendants("e21"));
}

TEST(FindByPath_EntityTest)
{
	Entity root;
	createTree(root);
	CHECK_EQUAL(&root, root.findEntityByPath(""));

	CHECK(!root.findEntityByPath("../"));

	CHECK_EQUAL(&root, root.findEntityByPath("./"));

	CHECK_EQUAL(&root, e1->findEntityByPath(".."));
	CHECK_EQUAL(&root, e1->findEntityByPath("../"));
	CHECK_EQUAL(&root, e11->findEntityByPath("../../"));

	CHECK_EQUAL(e1, root.findEntityByPath("e1"));
	CHECK_EQUAL(e11, root.findEntityByPath("e1/e11"));
	CHECK(!root.findEntityByPath("e1/not exist"));

	CHECK_EQUAL(e2, e1->findEntityByPath("../e2"));

	CHECK(!root.findEntityByPath("e11"));
}

TEST(CommonAncestor_EntityTest)
{
	Entity root;
	createTree(root);

	CHECK_EQUAL(e1, Entity::commonAncestor(*e13, *e11));
	CHECK_EQUAL(&root, Entity::commonAncestor(*e3, *e11));

	Entity dummy;
	CHECK(nullptr == Entity::commonAncestor(dummy, *e11));
}

TEST(GetRelativePath_EntityTest)
{
	Entity root;
	createTree(root);

	CHECK_EQUAL("", root.getRelativePathFrom(root));
	CHECK_EQUAL("", e1->getRelativePathFrom(*e1));

	CHECK_EQUAL("../", root.getRelativePathFrom(*e1));
	CHECK_EQUAL("../../", root.getRelativePathFrom(*e11));

	CHECK_EQUAL("e1/", e1->getRelativePathFrom(root));
	CHECK_EQUAL("e1/e11/", e11->getRelativePathFrom(root));

	CHECK_EQUAL("../e2/", e2->getRelativePathFrom(*e1));
	CHECK_EQUAL("../e1/", e1->getRelativePathFrom(*e2));

	CHECK_EQUAL("../../e1/e12/", e12->getRelativePathFrom(*e21));
	CHECK_EQUAL("../../e2/e21/", e21->getRelativePathFrom(*e12));

	// Test with 2 totally seperated tree
	Entity root2;
	CHECK_EQUAL("", root.getRelativePathFrom(root2));

	Entity* e = new Entity;
	e->asChildOf(&root2);
	CHECK_EQUAL("", e1->getRelativePathFrom(*e));
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
		sal_override Component* clone() const { return new CloneableComponent(*this); }
	};	// CloneableComponent

	class NonCloneableComponent : public Component
	{
	public:
		std::string data;

		NonCloneableComponent(const std::string& _data) : data(_data) {}

		sal_override const std::type_info& familyType() const { return typeid(NonCloneableComponent); }
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

	// Find the added component
	CHECK(e1->findComponent<CloneableComponent>());
	CHECK(e1->findComponent<NonCloneableComponent>());

	// Apply some transformations
	root.localTransform.setTranslation(Vec3f(1, 2, 3));
	e1->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver2(), 0));
	e13->localTransform.setTranslation(Vec3f(3, 2, 1));

	std::auto_ptr<Entity> clone_root(root.clone());
	Entity* clone_e1 = clone_root->findEntityInDescendants("e1");
	Entity* clone_e2 = clone_root->findEntityInDescendants("e2");
	Entity* clone_e3 = clone_root->findEntityInDescendants("e3");
	Entity* clone_e11 = clone_root->findEntityInDescendants("e11");
	Entity* clone_e12 = clone_root->findEntityInDescendants("e12");
	Entity* clone_e13 = clone_root->findEntityInDescendants("e13");

	{
		bool allNotNull = clone_e1 && clone_e2 && clone_e3 && clone_e11 && clone_e12 && clone_e13;
		CHECK(allNotNull);
		if(!allNotNull)
			return;
	}

	CHECK_EQUAL(clone_root->name, "root");
	CHECK_EQUAL(clone_e1->name, "e1");
	CHECK_EQUAL(clone_e2->name, "e2");
	CHECK_EQUAL(clone_e3->name, "e3");
	CHECK_EQUAL(clone_e11->name, "e11");
	CHECK_EQUAL(clone_e12->name, "e12");
	CHECK_EQUAL(clone_e13->name, "e13");

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

#include "../../../MCD/Core/Entity/BehaviourComponent.h"

// Ensure the reference structure from one component to another is preserved after clone
TEST(ComponentCrossReference_Clone_EntityTest)
{

typedef IntrusiveWeakPtr<class MockComponent> MockComponentPtr;

class MockComponent : public BehaviourComponent
{
public:
	sal_override sal_maybenull Component* clone() const { return new MockComponent(); }

	sal_override sal_checkreturn bool postClone(const Entity& src, Entity& dest)
	{
		// Find the Component in the src tree that corresponding to this
		MockComponent* srcComponent = dynamic_cast<MockComponent*>(
			ComponentPreorderIterator::componentByOffset(src, ComponentPreorderIterator::offsetFrom(dest, *this))
		);

		if(!srcComponent)
			return false;
		if(!srcComponent->referenceToAnother)
			return true;

		// Find the Component in the src tree that corresponding to referenceToAnother
		referenceToAnother = dynamic_cast<MockComponent*>(
			ComponentPreorderIterator::componentByOffset(dest, ComponentPreorderIterator::offsetFrom(src, *srcComponent->referenceToAnother))
		);

		return true;
	}

	sal_override void update(float dt) {}

	MockComponentPtr referenceToAnother;
};	// MockComponent

	Entity root;
	root.name = "root";

	e1 = new Entity();
	e1->asChildOf(&root);
	MockComponent* c1 = new MockComponent();
	e1->addComponent(c1);

	e2 = new Entity();
	e2->asChildOf(&root);
	MockComponent* c2 = new MockComponent();
	e2->addComponent(c2);

	c1->referenceToAnother = c2;
	c2->referenceToAnother = c1;

	// Find back the component
	CHECK(!e1->findComponent<MockComponent>());
	CHECK_EQUAL(c1, e1->findComponent<BehaviourComponent>());
	CHECK_EQUAL(c1, e1->findComponentExactType<MockComponent>());

	// Clone the entity tree
	std::auto_ptr<Entity> cloned(root.clone());

	// Verify that the new cloned tree has the desired component cross-reference.
	MockComponent *cc1 = nullptr, *cc2 = nullptr;
	size_t i = 0;
	for(ComponentPreorderIterator itr(cloned.get()); !itr.ended(); itr.next(), ++i) {
		if(i == 0) cc1 = dynamic_cast<MockComponent*>(itr.current());
		if(i == 1) cc2 = dynamic_cast<MockComponent*>(itr.current());
	}

	if(cc1 && cc2) {
		CHECK_EQUAL(cc2, cc1->referenceToAnother);
		CHECK_EQUAL(cc1, cc2->referenceToAnother);
	}
	else
		CHECK(false);
}
