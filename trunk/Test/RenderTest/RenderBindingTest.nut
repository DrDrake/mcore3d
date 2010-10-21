framework.initWindow("title=RenderBindingTest;width=800;height=600;fullscreen=0;FSAA=8")

local e = framework.guiLayer.addFirstChild(Entity("A text label"));
e.addComponent(TextLabelComponent());
e.localTransform.translation = Vec3(0, 300, 0);
e.TextLabelComponent.text = "Hello world";

// A layer for no-lighting material
local noLighting = framework.sceneLayer.addFirstChild(Entity("No lighting material"));

{	// Material without lighting
	local m = noLighting.addComponent(MaterialComponent());
	m.lighting = false;
	m.cullFace = false;
	m.useVertexColor = true;
}

framework.load("a.png");

{	// Build the xyz axis
	local axis = noLighting.addFirstChild(Entity("Axis"));
	local c = axis.addComponent(DisplayListComponent());

	c.beginLines();
		c.color(1, 0, 0);
		c.vertex(-1, 0, 0);
		c.vertex(10, 0, 0);

		c.color(0, 1, 0);
		c.vertex(0, -1, 0);
		c.vertex(0, 10, 0);

		c.color(0, 0, 1);
		c.vertex(0, 0, -1);
		c.vertex(0, 0, 10);
	c.end();
}

while(framework.update()) {
	e.TextLabelComponent.text = "Hello world" +  ", " + framework.fps;
}
