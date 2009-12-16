// For those Entity that were child of an Entity created from loadEntity(),
// we need to use a "patch" like method to modify them.
class PatchBlock
{
	function addString(str)
	{
		output += "\t";
		output += str;
	}

	function getString()
	{
		if(output == "")
			return "";
		else
			return preOutput + output + postOutput;
	}

	entity = null;	// The root entity of which the tree is going to create patch
	preOutput = "";	// Use to provide load sync handling ( using addResourceCallback() )
	output = "";
	postOutput = "";
}

// Hold information for serialization.
class SerializationState
{
	constructor()
	{
		reset();
	}

// Operations
	function reset()
	{
		varNameMap = {};
		varNameMap[rootEntity] <- "rootEntity";
		prefixCounter = {};
		pendingRef = {};
		output = "";

		patch = {};
		currentPatch = null;
	}

	// Generate unique variable name.
	function getObjName(obj, namePrefix)
	{
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
	function addReference(srcObj, varName, refObj)
	{
		local srcName = getObjName(srcObj, null);
		local str = ::format("\t%s.%s = %s;\n", srcName, varName, "%s");

		if(refObj in varNameMap)
			output += ::format(str, getObjName(refObj, null));
		else {
			assert(!(refObj in pendingRef));
			pendingRef[refObj] <- str;
		}
	}

	// Invoked by Component.serialize() or Entity.serialize()
	// such that any pending un-resolved reference will become resolved.
	function resolveReference(obj)
	{
		if(!(obj in pendingRef))
			return;
		output += ::format(pendingRef[obj], getObjName(obj, null));
		delete pendingRef[obj];
	}

	function beginPatch(entity, resourceFilePath)
	{
		local path = entity.getRelativePathFrom(rootEntity);
		if(!(path in patch))
		{
			currentPatch = PatchBlock();
			currentPatch.entity = entity;
			currentPatch.preOutput = ::format("addResourceCallback(\"%s\", function()\n{\n", resourceFilePath);
			currentPatch.postOutput = "});\n";
			patch.rawset(path, currentPatch);
		}
	}

	function endPatch(entity)
	{
		currentPatch = null;
	}

	function addPatchString(s)
	{
		if(currentPatch)
			currentPatch.addString(s);
	}

	function getPatchString()
	{
		local ret = "";
		foreach(k,v in patch) {
			ret += v.getString();
		}
		return ret;
	}

// Attributes
	varNameMap = {};	// Map between an object and it's corresponding variable name
	prefixCounter = {};	// Prefix is 'e' for Entity, 'c' for Component
	pendingRef = {};	// A map for tracking reference dependency problem
	output = "";

	patch = {};			// Map
	currentPatch = null;// Point to a value in "patch" as the current
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

	if(!entity.localTransform.isIdentity())
		state.output += ::format("%s.transform.fromHex(\"%s\");\n", thisVarName, entity.localTransform.toHex());

	// Components
	foreach(c in entity.components)
		try { c.serialize(state); } catch(err) { println(err); }

	state.output += "\n";
}

function writeOutputWithConstructionString(entity, state, constructionString)
{
	local parentVarName = state.getObjName(entity.parentNode, null);
	local thisVarName = state.getObjName(entity, "e");

	// Construction and parent-child relationship
	state.output += ::format("local %s = %s;\n", thisVarName, constructionString);
	state.output += ::format("%s.addChild(%s);\n", parentVarName, thisVarName);
	state.output += ::format("%s.name = \"%s\";\n", thisVarName, entity.name);

	// Entity's property
	if(!entity.enabled)
		state.output += ::format("%s.enabled = false;\n", thisVarName);

	if(!entity.localTransform.isIdentity())
		state.output += ::format("%s.transform.fromHex(\"%s\");\n", thisVarName, entity.localTransform.toHex());

	// Components
	foreach(c in entity.components)
		try { c.serialize(state); } catch(err) { println(err); }

	state.output += "\n";
}

function entityDeferSerializeTraverse(entity, state)
{
	if(!entity)
		return;

	entityDeferSerializeTraverse(entity.nextSibling, state);

	local transformDirty = !entity.localTransform.isIdentity();
	local enableDirty = !entity.enabled;

	if(transformDirty || enableDirty)
	{
		local srcEntity = state.currentPatch.entity;

		// NOTE: entity.name should equals to the last element of relativePath
		local relativePath = entity.getRelativePathFrom(srcEntity);
		local srcVarName = state.getObjName(srcEntity, null);
		local varName = state.getObjName(entity, "e");

		state.addPatchString(::format("local %s = %s.findEntityByPath(\"%s\");\n", varName, srcVarName, relativePath));

		if(transformDirty)
			state.addPatchString(::format("%s.localTransform.fromHex(\"%s\");\n", varName, entity.localTransform.toHex()));

		if(enableDirty)
			state.addPatchString(::format("%s.enabled = false;\n", varName));
	}

	// TODO: Write the string to the patch structure
//	state.resolveReference(entity);

	entityDeferSerializeTraverse(entity.firstChild, state);
}

// Traverse the Entity tree
function entityDirectSerializeTraverse(entity, state)
{
	if(!entity)
		return;

//	println(entity);
//	println(entity.name);

	if(entity.nextSibling)
		entity.nextSibling.serialize(state);

	// We skip the root entity
	if(entity != rootEntity) {
		writeOutput(entity, state);
		state.resolveReference(entity);
	}

	if(entity.firstChild)
		entity.firstChild.serialize(state);
}

gSerializationState <- SerializationState();
rootEntity.directSerialize = function(state) {
	::entityDirectSerializeTraverse(this, state);
}

// Convert a table to a string
// Reference: http://squirrel-lang.org/forums/thread/477.aspx
function tprint(table)
{
	return "{" + tprintex(table, 4, 4, "") + "}";
}

function tprintex(table, margin, tabwidth, newLine)
{
	local ret = "";
	local indent = function(n) : (ret) {
		for (local i = 0; i < n; ++i)
			ret += " ";
	}

	foreach(key, value in table)
	{
		indent(margin);
		ret += key;
		switch (type(value))
		{
		case "table":
			ret += (" = {" + newLine);
			ret += tprintex(value, margin + tabwidth, tabwidth);
			indent(margin);
			ret += "}";
			break;
		case "array":
			ret += (" = [" + newLine);
			ret += tprintex(value, margin + tabwidth, tabwidth);
			indent(margin);
			ret += "]";
			break;
		case "string":
			ret += " = \"";
			ret += value;
			ret += "\"";
			break;
		default:
			ret += " = ";
			ret += value;
			break;
		}
		ret += newLine;
	}

	return ret;
}

// Patch the loadEntity() funciton
{
	local original = ::loadEntity;
	loadEntity = function(filePath, loadOptions={}) : (original)
	{
		local e = original(filePath, loadOptions);
		if(e) e.directSerialize = function(state) : (e, filePath, loadOptions)
		{
			// TODO: Integrate with resource manager update() loop, to ensure the Entity is
			// fully loaded before serializing to file.
			addResourceCallback(filePath, function():(filePath) { println(filePath + " loaded."); });

			if(e.nextSibling)
				e.nextSibling.serialize(state);

			::writeOutputWithConstructionString(
				e, state,
				::format("loadEntity(\"%s\", %s)", filePath, tprint(loadOptions))
			);

			state.beginPatch(e, filePath);

			if(e.firstChild)
				e.firstChild.serialize(state);

			state.endPatch(e);
		};

		return e;
	}
}

// Registration for various components

PickComponent.classString <- "PickComponent()";
PickComponent.serialize <- function(state)
{
	::Component.serialize(state);
	state.addReference(this, "entityToPick", entityToPick);
}
