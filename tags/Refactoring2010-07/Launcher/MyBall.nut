return class extends RigidBodyComponent
{
	constructor(mass, shape)
	{
		::RigidBodyComponent.constructor(mass, shape);
	}

	function update()
	{
		sleep(4);
//		local e = entity;
//		e.transform.translateBy(Vec3(0, 5, 0));
//		::println("hi");
	}
}
