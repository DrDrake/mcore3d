framework.initWindow("title=RenderBindingTest;width=800;height=600;fullscreen=0;FSAA=8");
framework.addFileSystem("./Media");

local e = framework.guiLayer.addFirstChild("A text label");
e.addComponent(TextLabelComponent);
e.localTransform.translation = Vec3(0, 500, 0);
e.TextLabelComponent.text = "Hello world";
e.TextLabelComponent.anchor = Vec2(0, 1);

// A layer for no-lighting material
local noLighting = framework.sceneLayer.addFirstChild("No lighting material");

{	// Material without lighting
	local m = noLighting.addComponent(MaterialComponent);
	m.lighting = false;
	m.cullFace = false;
	m.useVertexColor = true;
}

framework.load("a.png");

{	// Build the xyz axis
	local axis = noLighting.addFirstChild("Axis");
	local c = axis.addComponent(DisplayListComponent);

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

class MyScript extends ScriptComponent
{
	function update(dt) {
		::println("hi");
		entity.destroyThis();
	}
}

framework.sceneLayer.addFirstChild("Script").addComponent(MyScript);

class DrawHistogram extends ScriptComponent
{
	time = 0;
	static refershRate = 60;

	function func(x) {
		return ::sin(0.5 * x) * 10;
	}

	function update(dt)
	{
		local segments = 100;
		local min = -12.5;
		local max = 12.5;
		local dx = (max - min) / segments;

		local d = entity.DisplayListComponent;
		d.clear();
		d.beginTriangles();
			d.color(0, 0.5, 0);

			for(local x = min; x <= max; x += dx) {
				local y = func(x + time);
				local y2 = func(x + dx + time);
				d.vertex(x, 0, 0);
				d.color(0, 1, 0);
				d.vertex(x, y, 0);
				d.color(0, 0.5, 0);
				d.vertex(x+dx, 0, 0);

				d.vertex(x+dx, 0, 0);
				d.vertex(x, y, 0);
				d.vertex(x+dx, y2, 0);
			}
		d.end();

		sleep(0);	// Effectively do nothing
		sleep(1.0/refershRate);
		time += 1.0/refershRate;
	}
}

local histogam = noLighting.addFirstChild("Histogram");
histogam.addComponent(DrawHistogram);
histogam.addComponent(DisplayListComponent);
