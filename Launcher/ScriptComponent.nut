// Something similar to this script file context can be found at:
// http://squirrel-lang.org/forums/thread/2168.aspx

return class extends ScriptComponent
{
	constructor() {
		// NOTE: This is very important to call the base class constructor,
		// otherwise null pointer dereference occur and crash!
		::ScriptComponent.constructor();

		hp = 100;
		position = ::Vec3(1, 0, 0);
		speed = ::Vec3(0.5, 0.5, 0);
	}

	function update() {
		position.addEqual(speed);
		local v = ::Vec3(0);
//		if(::gInput.anyKey())
//			println(position);
		if(::gInput.getMouseButton(0))
			println(gInput.mousePosition);
		if(::gInput.getButtonDown("w"))
			println("up");
		if(::gInput.getButtonDown("a"))
			println("left");
		if(::gInput.getButtonDown("s"))
			println("down");
		if(::gInput.getButtonDown("d"))
			println("right");
	}

	hp = 100;
	position = null;
	speed = null;
}
