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

function writeOutput(entity, state)
{
	local parentVarName = state.getObjName(entity.parentNode, null);
	local thisVarName = state.getObjName(entity, "e");

	// Construction and parent-child relationship
	state.output += ::format("local %s = Entity(\"%s\");\n", thisVarName, entity.name);
	state.output += ::format("%s.addChild(%s);\n", parentVarName, thisVarName);

	// Entity's property
	if(!entity.enabled)
		state.output += ::format("%s.enabled = false;\n", thisVarName);

	// Components
	foreach(c in entity.components)
		try { c.serialize(state); } catch(err) {}

	state.output += "\n";
}

function writeOutputWithConstructionString(entity, state, constructionString)
{
	local parentVarName = state.getObjName(entity.parentNode, null);
	local thisVarName = state.getObjName(entity, "e");

	// Construction and parent-child relationship
	state.output += ::format("local %s = %s;\n", thisVarName, constructionString);
	state.output += ::format("%s.addChild(%s);\n", parentVarName, thisVarName);

	// Entity's property
	if(!entity.enabled)
		state.output += ::format("%s.enabled = false;\n", thisVarName);

	// Components
	foreach(c in entity.components)
		try { c.serialize(state); } catch(err) {}

	state.output += "\n";
}

// Traverse the Entity tree
function entitySerializeTraverse(entity, state)
{
	if(!entity)
		return;

	try { entity.nextSibling.serialize(state); } catch(err) {}

	// We skip the root entity
	if(entity != rootEntity)
		writeOutput(entity, state);

	try { entity.firstChild.serialize(state); } catch(err) {}
}

gSerializationState <- SerializationState();
rootEntity.serialize = Entity._serialize;

// Registration for various components

PickComponent.classString <- "PickComponent()";
PickComponent.serialize <- function(state)
{
	::Component.serialize(state);
	local name = state.getObjName(this, null);
	state.addReference(this, "entityToPick", entityToPick);
}
