c1 <- loadComponent("ScriptComponent.nut");
c2 <- loadComponent("ScriptComponent.nut");

assert(c1.entity == null);
rootEntity.addComponent(c1);
assert(c1.entity != null);

local e = Entity();
rootEntity.addChild(e);
e.addComponent(c2);

e = loadEntity("Scene/City/scene.3ds");
rootEntity.addChild(e);

e = loadEntity("Scene/03/scene.3ds");
rootEntity.addChild(e);

return {
	name = "root Entity",
	component = [loadComponent("ScriptComponent.nut")]
	children = {}
};
