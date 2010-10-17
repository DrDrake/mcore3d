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

	function testDestroy()
	{
		e1.destroyThis();

		assert(!e1.parent);
//		assertEquals(e1, e11.parent);
//		e1 = null;
//		println(e11.parent.name);
//		assert(!e11.parent);
	}

	function testProperty()
	{
		root.enabled = true;
		root.name = "Root node";
		root.localTransform = Mat44();
	}
}

SqUnit().run();
