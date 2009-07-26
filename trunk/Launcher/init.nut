c1 <- loadComponent("ScriptComponent.nut");
c2 <- loadComponent("ScriptComponent.nut");

assert(c1.entity == null);
rootEntity.addComponent(c1);
assert(c1.entity != null);

local e = Entity();
rootEntity.addChild(e);
e.addComponent(c2);

{	// Load the scene
	local scene1 = "Scene/City/scene.3ds";
	e = loadEntity(scene1, {createStaticRigidBody=true});
	addResourceCallback(scene1, function():(scene1) { println("loaded: " + scene1); });
	rootEntity.addChild(e);

	local scene2 = "Scene/03/scdene.3ds";
	e = loadEntity(scene2);
	addResourceCallback(scene2, function():(scene2) { println("loaded: " + scene2); });
	rootEntity.addChild(e);

	// Prints a message when both resources are loaded.
	addResourceCallback([scene1,scene2], function() { println("all scene loaded"); });
}

// Create some spheres
for(local i=0; i<20; ++i) for(local j=0; j<20; ++j)
{
	local e = Entity();
	e.localTransform.m03 = 5 * i;
	e.localTransform.m13 = 100;
	e.localTransform.m23 = 5 * j;
	rootEntity.addChild(e);

	local c = MeshComponent();
	c.mesh= ChamferBoxMesh(1, 5);
	e.addComponent(c);

	c = RigidBodyComponent(gMainWindow.dynamicsWorld, 1, SphereShape(1));
	e.addComponent(c);
}
