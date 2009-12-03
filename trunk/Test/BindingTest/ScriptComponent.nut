// Something similar to this script file context can be found at:
// http://squirrel-lang.org/forums/thread/2168.aspx

return class extends ScriptComponent
{
	constructor() {
		// NOTE: This is very important to call the base class constructor,
		// otherwise null pointer dereference occur and crash!
		::ScriptComponent.constructor();

		hp = 100;
		position = Vec3(1, 0, 0);
		speed = Vec3(0.5, 0.5, 0);
	}

	function update() {
		position.addEqual(speed);
//		println(position);
	}

	function serialize(state) {
		::ScriptComponent.serialize(state);
		local name = state.getObjName(this, null);
		state.output += ::format("\t%s.hp = %i;\n", name, hp);
		state.output += ::format("\t%s.position.fromHex(\"%s\");\n", name, position.toHex());
		state.output += ::format("\t%s.speed.fromHex(\"%s\");\n", name, speed.toHex());
		state.addReference(this, "anotherComponent", anotherComponent);
	}

	hp = null;
	position = null;
	speed = null;
	anotherComponent = null;

	static classString = "::loadComponent(ScriptComponent.nut)";
}
