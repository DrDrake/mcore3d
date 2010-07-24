{	// Setup the falling balls

	// Multiple MeshComponent can share a single Mesh
	local chamferBoxMesh = ChamferBoxMesh(1, 2);

	local ballEffect = loadResource("Material/test.fx.xml");

	// Multiple RigidBodyComponent can share a single shape
	local shpereShape = SphereShape(1);

	// Create some spheres
	for(local i=0; i<30; ++i) for(local j=0; j<30; ++j)
	{
		local e = Entity();
		e.localTransform.translation = Vec3(i, 100, j);
		rootEntity.addChild(e);

		local c = MeshComponent();
		c.mesh = chamferBoxMesh;
		c.effect = ballEffect;
		e.addComponent(c);

		c = loadComponent("MyBall.nut", 1, shpereShape);
//		c = RigidBodyComponent(1, shpereShape);
		e.addComponent(c);
	}
}

{	// Setup for the camera
	local e = Entity();

	local camera = loadComponent("FpsCamera.nut");
	e.name= "Fps camera";
	e.addComponent(camera);

	rootEntity.addChild(e);
}
