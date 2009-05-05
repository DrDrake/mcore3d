function println(str) {
	print(str);
	print("\n");
}

local root = Entity.create();
assert(root.parentNode == null);
root.enabled = true;
root.name = "Root node";

local n1 = Entity.create();
root.addChild(n1);
n1.name = "Child node 1";
n1.localTransform = Mat44.create();
assert(root.name == "Root node");
assert(root.firstChild.name == "Child node 1");

local camera = CameraComponent.create(n1);
assert(n1.name == camera.entity.name);
println(camera.velocity);
assert(camera.isValid());
n1.unlink();
assert(!camera.isValid());
assert(!camera.entity);
