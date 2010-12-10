framework.initWindow("title=TheEarth;width=800;height=600;fullscreen=0;FSAA=8");
framework.addFileSystem("./Media");

local scene = framework.sceneLayer;

// Create a sphere mesh
local sphereMeshBuffer = ::createSphere(8, 30);

local shells = scene.addFirstChild("Sphere shells");

{	// Inner shell for the ground
	local e = shells.addFirstChild("Inner shell");

	// Earth textures from:
	// http://www.oera.net/How2/TextureMaps2.htm?src=mappery
	local material = e.addComponent(StandardMaterial());
	material.diffuseMap = "Earth/Diffuse.jpg";
	material.emissionMap = "Earth/Emission.jpg";
	material.bumpMap = "Earth/Bump.jpg";
	material.specularMap = "Earth/Specular.jpg";
	material.bumpFactor = 0.1;

	e = e.addFirstChild("");
	e.addComponent(Mesh()).buffer = sphereMeshBuffer;
}

{	// Outer shell for the air
	local e = shells.addFirstChild("Outer shell");

	local material = e.addComponent(StandardMaterial());
	material.alphaMap = "Earth/Clouds.jpg";
	material.specularColor = ColorRGBA(0, 0, 0, 0);
	material.opacity = 0.8;

	e = e.addFirstChild("");
	e.addComponent(Mesh()).buffer = sphereMeshBuffer;
	e.localTransform.scaleBy(Vec3(1.01));
}

// Find out the default camera and move it backward
foreach(e in scene.descendants) {
	if(e.hasComponent(Camera)) {
		e.localTransform.translateBy(Vec3(0, 0, 10));
		break;
	}
}
