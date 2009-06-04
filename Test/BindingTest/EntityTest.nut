dofile("squnit.nut", true);

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
		root = Entity();
		root.name = "root";

		e1 = Entity();
		e1.name = "e1";
		root.addChild(e1);

		e2 = Entity();
		e2.name = "e2";
		root.addChild(e2);

		e3 = Entity();
		e3.name = "e3";
		root.addChild(e3);

		e11 = Entity();
		e11.name = "e11";
		e1.addChild(e11);
		
		e12 = Entity();
		e12.name = "e12";
		e1.addChild(e12);

		e13 = Entity();
		e13.name = "e13";
		e1.addChild(e13);

		e21 = Entity();
		e21.name = "e21";
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
}

SqUnit().run();
return;


local root = Entity();
assert(root.parentNode == null);
root.enabled = true;
root.name = "Root node";

local n1 = Entity();
root.addChild(n1);
n1.name = "Child node 1";
n1.localTransform = Mat44();
assert(root.name == "Root node");
assert(root.firstChild.name == "Child node 1");

local camera = CameraComponent();
n1.addComponent(camera);
assert(n1.name == camera.entity.name);
println(camera.velocity);
assert(camera);
n1.unlink();
assert(camera);
n1 = null;
assert(!camera);
