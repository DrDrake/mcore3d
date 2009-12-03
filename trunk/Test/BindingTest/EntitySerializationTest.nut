c1 <- loadComponent("ScriptComponent.nut");

rootEntity.addComponent(c1);
rootEntity.name = "root";

{	local e = Entity();
	e.name = "entity1";
	rootEntity.addChild(e);
	local c1 = loadComponent("ScriptComponent.nut");
	e.addComponent(c1);

	local e2 = Entity();
	e2.name = "entity1.1";
	local c2 = loadComponent("ScriptComponent.nut");
	e2.addComponent(c2);
	c2.hp = 50;
	e.addChild(e2);

	local e3 = Entity();
	e3.name = "entity1.2";
	e.addChild(e3);

	// Create some cyclic reference
	c1.anotherComponent = c2;
	c2.anotherComponent = c1;
}

{	local e = Entity();
	e.name = "entity2";
	rootEntity.addChild(e);

	local e2 = Entity();
	e2.name = "entity2.1";
	e.addChild(e2);
	e2.enabled = false;
}

//for(local i=0; i<10; ++i)
//	updateAllScriptComponent();

class SerializationState
{
	constructor() {
		varNameMap[rootEntity] <- "rootEntity";
	}

	function getObjName(obj, namePrefix) {
		if(obj in varNameMap)
			return varNameMap[obj];
		assert(namePrefix != null);

		if(namePrefix in prefixCounter)
			prefixCounter[namePrefix]++;
		else
			prefixCounter[namePrefix] <- 1;

		local varName = namePrefix + prefixCounter[namePrefix];
		varNameMap[obj] <- varName;
		return varName;
	}

	// When an object needs to reference another serialized object, 
	// use this function to track upon the referencing relationship,
	// such that we can handle complex object graph even with cycle-reference.
	// Parameter usage: srcObj.varName = refObj
	function addReference(srcObj, varName, refObj) {
		local srcName = getObjName(srcObj, null);
		local str = ::format("\t%s.%s = %s;\n", srcName, varName, "%s");

		if(refObj in varNameMap)
			output += ::format(str, getObjName(refObj, null));
		else {
			assert(!(refObj in pendingRef));
			pendingRef[refObj] <- str;
		}
	}

	function resolveReference(obj) {
		if(!(obj in pendingRef))
			return;
		output += ::format(pendingRef[obj], getObjName(obj, null));
		delete pendingRef[obj];
	}

	varNameMap = {};	// Map between an object and it's corresponding variable name
	prefixCounter = {};	// Prefix is 'e' for Entity, 'c' for Component
	pendingRef = {};	// A map for tracking reference dependency problem
	output = "";
}

state <- SerializationState();

function writeOutput(entity)
{
	local parentVarName = state.getObjName(entity.parentNode, null);
	local thisVarName = state.getObjName(entity, "e");

	// Construction and parent-child relationship
	state.output += ::format("local %s = Entity();\n", thisVarName);
	state.output += ::format("%s.addChild(%s);\n", parentVarName, thisVarName);

	// Entity's property
	if(entity.name != "")
		state.output += ::format("%s.name = \"%s\"\n", thisVarName, entity.name);

	if(!entity.enabled)
		state.output += ::format("%s.enabled = false;\n", thisVarName);

	// Components
	foreach(c in entity.components)
		c.serialize(state);

	state.output += "\n";
}

// Traverse the Entity tree
function traverse(entity)
{
	if(!entity)
		return;

	traverse(entity.nextSibling);

	// We skip the root entity
	if(entity != rootEntity)
		writeOutput(entity);

	traverse(entity.firstChild);
}

traverse(rootEntity);

//println(state.output);
