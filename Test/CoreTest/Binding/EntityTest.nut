class TestEntity
{
	root = null;
	e1 = null;
	e2 = null;
	e3 = null;
	e11 = null;
	e12 = null;
	e13 = null;
	e21 = null;

	function setUp()
	{
		root = Entity("root");

		e1 = root.addLastChild(Entity("e1"));
		e2 = root.addLastChild(Entity("e2"));
		e3 = root.addLastChild(Entity("e3"));

		e11 = e1.addLastChild(Entity("e11"));
		e12 = e1.addLastChild(Entity("e12"));
		e13 = e1.addLastChild(Entity("e13"));

		e21 = e2.addLastChild(Entity("e21"));
	}

	function testHierarchy()
	{
		assert(!root.parent);
		assert(!root.nextSibling);
		assertEquals(e1, root.firstChild);

		assertEquals(e1.nextSibling, e2);
		assertEquals(e2.nextSibling, e3);
		assert(!e3.nextSibling);
	}

	function testMemoryReference()
	{
		assertEquals(root.firstChild, e1);
		assertEquals(root.firstChild, root.firstChild);
		e1 = null;	// Now no more script object reference e1

		// Get back e1 from root
		e1 = root.firstChild;
		assertEquals(e1.name, "e1");
	}

	function testDestroyLeaf()
	{
		e21.destroyThis();

		assert(!e21.parent);
		assert(!e21.firstChild);
		assert(!e21.nextSibling);

		assertEquals(root, e2.parent);
		assert(!e2.firstChild);
		assertEquals(e1.nextSibling, e2);
	}

	function testDestroyFirstSlibing()
	{
		e1.destroyThis();

		assert(!e1.parent);
		assert(!e1.firstChild);
		assert(!e1.nextSibling);

		assertEquals(e2, root.firstChild);
	}

	function testDestroyMiddleSlibing()
	{
		assert(e1.nextSibling);
		e2.destroyThis();

		assert(!e2.parent);
		assert(!e2.firstChild);
		assert(!e2.nextSibling);

		// When e2 is destroy, e21 is also destroyed
		assert(!e21.parent);
		assert(!e21.firstChild);
		assert(!e21.nextSibling);

		assertEquals(e1.nextSibling, e3);
	}

	function testDestroyLastSlibing()
	{
		assert(e2.nextSibling);
		e3.destroyThis();

		assert(!e3.parent);
		assert(!e3.firstChild);
		assert(!e3.nextSibling);

		assert(!e2.nextSibling);
	}

	function testProperty()
	{
		root.enabled = true;
		root.name = "Root node";
		root.localTransform = Mat44();
	}
}

//SqUnit().runTestMethod("TestEntity.testDestroy", TestEntity, TestEntity.testDestroy);
SqUnit().run();
