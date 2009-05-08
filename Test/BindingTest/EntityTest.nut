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
//	assert(e3 == root.firstChild);
}
//hierarchyTest();

function unlinkTest()
{
	local root = createTree();
	e3 = e3.unlink();

	assert(e3.parentNode == null);
}
unlinkTest();

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
