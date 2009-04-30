#include "Pch.h"
#include "../../../MCD/Core/Entity/Entity.h"

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
	CHECK(!root.nextSlibing());
	CHECK_EQUAL(e3, root.firstChild());

	CHECK_EQUAL(&root, e1->parent());
	CHECK_EQUAL(&root, e2->parent());
	CHECK_EQUAL(&root, e3->parent());

	// The slibings are in reverse order
	CHECK_EQUAL(e2, e3->nextSlibing());
	CHECK_EQUAL(e1, e2->nextSlibing());
	CHECK(!e1->nextSlibing());
}

TEST(Unlink_EntityTest)
{
	{	// Unlink a leaf node
		Entity root;
		createTree(root);
		e21->unlink();

		CHECK(!e21->parent());
		CHECK(!e21->firstChild());
		CHECK(!e21->nextSlibing());

		CHECK_EQUAL(&root, e2->parent());
		CHECK(!e2->firstChild());
		CHECK_EQUAL(e1, e2->nextSlibing());

		delete e21;
	}

	{	// Unlink first slibing node
		Entity root;
		createTree(root);
		e3->unlink();

		CHECK(!e3->parent());
		CHECK(!e3->firstChild());
		CHECK(!e3->nextSlibing());

		CHECK_EQUAL(e2, root.firstChild());

		delete e3;
	}

	{	// Unlink middle slibing node
		Entity root;
		createTree(root);
		e2->unlink();

		CHECK(!e2->parent());
		CHECK_EQUAL(e21, e2->firstChild());
		CHECK(!e2->nextSlibing());

		CHECK_EQUAL(e1, e3->nextSlibing());

		delete e2;
	}

	{	// Unlink last slibing node
		Entity root;
		createTree(root);
		e1->unlink();

		CHECK(!e1->parent());
		CHECK_EQUAL(e13, e1->firstChild());
		CHECK(!e1->nextSlibing());

		CHECK(!e2->nextSlibing());

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
		CHECK_EQUAL(e3, e11->nextSlibing());
		CHECK_EQUAL(e11, root.firstChild());
	}

	{	// Insert before a non-first child
		Entity root;
		createTree(root);
		e11->insertBefore(e2);
		CHECK(!e11->firstChild());
		CHECK_EQUAL(&root, e11->parent());
		CHECK_EQUAL(e2, e11->nextSlibing());
	}

	{	// Insert after
		Entity root;
		createTree(root);
		e11->insertAfter(e2);
		CHECK(!e11->firstChild());
		CHECK_EQUAL(&root, e11->parent());
		CHECK_EQUAL(e1, e11->nextSlibing());
	}
}

TEST(WorldTransform_EntityTest)
{
	Entity root;
	createTree(root);

	root.localTransform.setTranslation(Vec3f(1, 2, 3));
	e1->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver2(), 0));
	e13->localTransform.setTranslation(Vec3f(3, 2, 1));

	Vec3f v(0.0f);
	e13->worldTransform().transformPoint(v);

	CHECK(v.isNearEqual(Vec3f(2, 4, 0)));
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
		CHECK_EQUAL(sizeof(expected)/sizeof(Entity*), i);
	}

	{	// Traversing a sub-tree only
		size_t i = 0;
		Entity* expected[] = { e1, e13, e12, e11 };

		for(EntityPreorderIterator itr(e1); !itr.ended(); itr.next(), ++i) {
			CHECK_EQUAL(expected[i], itr.operator->());
		}
		CHECK_EQUAL(sizeof(expected)/sizeof(Entity*), i);
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
