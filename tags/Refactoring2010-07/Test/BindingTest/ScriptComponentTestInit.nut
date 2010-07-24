c1 <- loadComponent("ScriptComponent.nut");
c2 <- loadComponent("ScriptComponent.nut");

assert(c1.entity == null);
rootEntity.addComponent(c1);
assert(c1.entity != null);

local e = Entity("");
rootEntity.addChild(e);
e.addComponent(c1);

for(local i=0; i<10000; ++i)
	updateAllScriptComponent();
