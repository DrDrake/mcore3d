function println(str) {
	print(str);
	print("\n");
}

e1 <- null;
e2 <- null;
e3 <- null;
e11 <- null;
e12 <- null;
e13 <- null;
e21 <- null;
function createTree()
{
	local root = Entity();
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

	return root;
}

function hierarchyTest()
{
	local root = createTree();

	assert(!root.parentNode);
	assert(!root.nextSlibing);
	assert(e3 == root.firstChild);

	// The slibings are in reverse order
	assert(e2 == e3.nextSlibing);
	assert(e1 == e2.nextSlibing);
	assert(!e1.nextSlibing);
}

function memoryReferenceTest()
{
	local root = createTree();

	assert(root.firstChild == e3);
	assert(root.firstChild == root.firstChild);
	e3 = null;	// Now no more script object reference e3

	{	// Make some noise
		local root2 = createTree();
		e3 = null;
	}

	assert(root.firstChild == root.firstChild);
	collectgarbage();
}

function unlinkTest()
{
	{	// Unlink a leaf node
		local root = createTree();
		e21.unlink();

		assert(!e21.parentNode);
		assert(!e21.firstChild);
		assert(!e21.nextSlibing);

		assert(root == e2.parentNode);
		assert(!e2.firstChild);
		assert(e1 == e2.nextSlibing);
	}

	{	// Unlink first slibing node
		local root = createTree();
		e3.unlink();

		assert(!e3.parentNode);
		assert(!e3.firstChild);
		assert(!e3.nextSlibing);

		assert(e2 == root.firstChild);
	}

	{	// Unlink middle slibing node
		local root = createTree();
		e2.unlink();

		assert(!e2.parentNode);
		assert(e21 == e2.firstChild);	// When e2 is destroy, e21 is also destroyed
		assert(!e2.nextSlibing);

		assert(e1 == e3.nextSlibing);
	}

	{	// Unlink last slibing node
		local root = createTree();
		e1.unlink();

		assert(!e1.parentNode);
		assert(e13 == e1.firstChild);
		assert(!e1.nextSlibing);
		assert(!e2.nextSlibing);
	}
}

function insertionTest()
{
	{	// Insert before a first child
		local root = createTree();
		e11.insertBefore(e3);
		assert(!e11.firstChild);
		assert(root == e11.parentNode);
		assert(e3 == e11.nextSlibing);
		assert(e11 == root.firstChild);
	}

	{	// Insert before a non-first child
		local root = createTree();
		e11.insertBefore(e2);
		assert(!e11.firstChild);
		assert(root == e11.parentNode);
		assert(e2 == e11.nextSlibing);
	}

	{	// Insert after
		local root = createTree();
		e11.insertAfter(e2);
		assert(!e11.firstChild);
		assert(root == e11.parentNode);
		assert(e1 == e11.nextSlibing);
	}
}

hierarchyTest();
memoryReferenceTest();
unlinkTest();
insertionTest();

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

local camera = CameraComponent(n1);
assert(n1.name == camera.entity.name);
println(camera.velocity);
assert(camera.isValid());
n1.unlink();
assert(!camera.isValid());
assert(!camera.entity);
