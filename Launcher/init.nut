c1 <- loadComponent("ScriptComponent.nut");
c2 <- loadComponent("ScriptComponent.nut");

assert(c1.entity == null);
rootEntity.addComponent(c1);
assert(c1.entity != null);

local e = Entity();
rootEntity.addChild(e);
e.addComponent(c2);

local scene1 = "Scene/City/scene.3ds";
e = loadEntity(scene1);
addResourceCallback(scene1, function():(scene1) { println("loaded: " + scene1); });
rootEntity.addChild(e);

local scene2 = "Scene/03/scdene.3ds";
e = loadEntity(scene2);
addResourceCallback(scene2, function():(scene2) { println("loaded: " + scene2); });
rootEntity.addChild(e);

// Prints a message when both resources are loaded.
addResourceCallback([scene1,scene2], function() { println("all scene loaded"); });
