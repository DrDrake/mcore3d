dofile("squnit.nut", true);

Entity.scriptVar <- null;

// TODO: Don't know why extend from ScriptComponent will make clone not work
class MyComponent extends MeshComponent
{
	constructor() {
		MeshComponent.constructor();
		name = "MyComponent";
	}
	name = null;
}

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

		e1 = Entity("e1");
		root.addChild(e1);

		e2 = Entity("e2");
		root.addChild(e2);

		e3 = Entity("e3");
		root.addChild(e3);

		e11 = Entity("e11");
		e1.addChild(e11);
		
		e12 = Entity("e12");
		e1.addChild(e12);

		e13 = Entity("e13");
		e1.addChild(e13);

		e21 = Entity("e21");
		e2.addChild(e21);
	}

	function testHierarchy()
	{
		assert(!root.parentNode);
		assert(!root.nextSibling);
		assertEquals(e3, root.firstChild);

		// The slibings are in reverse order
		assertEquals(e2, e3.nextSibling);
		assertEquals(e1, e2.nextSibling);
		assert(!e1.nextSibling);
	}

	function testMemoryReference()
	{
		assertEquals(root.firstChild, e3);
		assertEquals(root.firstChild, root.firstChild);
		e3 = null;	// Now no more script object reference e3

		local rootBackup = root;
		{	// Make some noise
			setUp();
			e3 = null;
		}

		assertEquals(rootBackup.firstChild, rootBackup.firstChild);
		::collectgarbage();
	}

	function testUnlinkLeaf()
	{
		// Unlink a leaf node
		e21.unlink();

		assert(!e21.parentNode);
		assert(!e21.firstChild);
		assert(!e21.nextSibling);

		assertEquals(root, e2.parentNode);
		assert(!e2.firstChild);
		assertEquals(e1, e2.nextSibling);
	}

	function testUnlinkLeaf()
	{
		e21.unlink();

		assert(!e21.parentNode);
		assert(!e21.firstChild);
		assert(!e21.nextSibling);

		assertEquals(root, e2.parentNode);
		assert(!e2.firstChild);
		assertEquals(e1, e2.nextSibling);
	}

	function testUnlinkFirstSlibing()
	{
		e3.unlink();

		assert(!e3.parentNode);
		assert(!e3.firstChild);
		assert(!e3.nextSibling);

		assertEquals(e2, root.firstChild);
	}

	function testUnlinkMiddleSlibing()
	{
		e2.unlink();

		assert(!e2.parentNode);
		assertEquals(e21, e2.firstChild);	// When e2 is destroy, e21 is also destroyed
		assert(!e2.nextSibling);

		assertEquals(e1, e3.nextSibling);
	}

	function testUnlinkLastSlibing()
	{
		e1.unlink();

		assert(!e1.parentNode);
		assertEquals(e13, e1.firstChild);
		assert(!e1.nextSibling);
		assert(!e2.nextSibling);
	}

	function testInsertionBefore()
	{	// Insert before a first child
		e11.insertBefore(e3);
		assert(!e11.firstChild);
		assertEquals(root, e11.parentNode);
		assertEquals(e3, e11.nextSibling);
		assertEquals(e11, root.firstChild);
	}

	function testInsertionBeforeNonFirst()
	{	// Insert before a non-first child
		e11.insertBefore(e2);
		assert(!e11.firstChild);
		assertEquals(root, e11.parentNode);
		assertEquals(e2, e11.nextSibling);
	}

	function testInsertionAfter()
	{	// Insert after
		e11.insertAfter(e2);
		assert(!e11.firstChild);
		assertEquals(root, e11.parentNode);
		assertEquals(e1, e11.nextSibling);
	}

	function testProperty()
	{
		root.enabled = true;
		root.name = "Root node";
		root.localTransform = Mat44();
	}

	function testScriptOwnershipHandle()
	{
		{	// Should not create memory leak
			local e = Entity();
		}

		{	// Any script side member variables assigned to Entity
			// should be persisted even crossing the script/cpp boundary.
			// This feature is made possible by using scriptOwnershipHandle.useStrongReference()
			local root = Entity();
			local e = Entity();
			e.scriptVar = 123;
			root.addChild(e);
			e = null;	// No more reference to Entity: 'e' on the script side.
			// But root.firstChild should already kept a strong reference to the script object after addChild().
			assertEquals(123, root.firstChild.scriptVar);
		}

		{	// Moving an Entity from one place to another should kept it's strong reference
			local root = Entity();
			local e = Entity("1");
			e.scriptVar = 123;
			root.addChild(e);
			e = Entity("2");
			e.scriptVar = 456;
			root.addChild(e);
			e = null;

			assertEquals(456, root.firstChild.scriptVar);
			assertEquals(123, root.firstChild.nextSibling.scriptVar);

			root.firstChild.addChild(root.firstChild.nextSibling);
			assertEquals(123, root.firstChild.firstChild.scriptVar);
		}

		{	// The cpp side should release the strong ownership once
			// the Entity is unlinked
			local root = Entity();
			local e = Entity();
			root.addChild(e);
			e = null;
			root.firstChild.unlink();
		}
	}

	function testComponentClone()
	{
		local c = MyComponent();
		c.name = "abcde";

		local c2 = clone c;
		assert(c != c2);
		assertEquals("abcde", c2.name);
	}

	function testEntityClone()
	{
		local root = Entity("root");
		
		{	// Setup the entities and components
			local mc = MyComponent();
			root.addComponent(mc);
			root.scriptVar = 123;

			local e = Entity("child");
			e.scriptVar = 456;
			root.addChild(e);
		}

		local cloned = clone root;
		assert(cloned && cloned != root);
		assertEquals(root.name, cloned.name);
		assertEquals(root.scriptVar, cloned.scriptVar);

		{	// Test the clonning of Entity's components
			local c1, c2;
			foreach(i,c in root.components)
				c1 = c;
			foreach(i,c in cloned.components)
				c2 = c;

			assert(c1);
			assert(c2);
			assertEquals(c1.name, c2.name);
			assertEquals(root, c1.entity);
			assertEquals(cloned, c2.entity);
		}

		assert(root.firstChild);
		assert(cloned.firstChild);
		assert(root.firstChild != cloned.firstChild);
		assertEquals(root.firstChild.name, cloned.firstChild.name);
		assertEquals(root.firstChild.scriptVar, cloned.firstChild.scriptVar);
	}

	function testComponentsIteration()
	{
		local e = Entity("");
		local c = [ ScriptComponent(), MeshComponent() ];
		c[0]._setScriptHandle();
		c[1]._setScriptHandle();
		e.addComponent(c[0]);
		e.addComponent(c[1]);

		local count = 0;
		foreach(i,ic in e.components) {
			assert(c[i] == ic);
			++count;
		}

		assert(count == 2);
	}
}

SqUnit().run();
