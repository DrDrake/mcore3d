#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/DisplayList.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Timer.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

namespace {

class DrawAxisComponent : public DisplayListComponent
{
public:
	DrawAxisComponent()
	{
		begin(DisplayListComponent::Lines);
			color(1, 0, 0);
			vertex(-1, 0, 0);
			vertex(10, 0, 0);

			color(0, 1, 0);
			vertex(0, -1, 0);
			vertex(0, 10, 0);

			color(0, 0, 1);
			vertex(0, 0, -1);
			vertex(0, 0, 10);
		end();
	}
};	// DrawAxisComponent

class LineCircleComponent : public DisplayListComponent
{
public:
	LineCircleComponent()
	{
		const float c = 3.14159f * 2;
		const size_t segments = 20;

		begin(DisplayListComponent::Lines);
			color(0.8f, 0.8f, 0.8f);
			for(size_t i=0; i<=segments; ++i)
				vertex(sinf(c*i/segments), 0, cosf(c*i/segments));
		end();

		begin(DisplayListComponent::LineStrip);
			color(0.5f, 0.5f, 0.5f);
			for(size_t i=0; i<=segments; ++i)
				vertex(0.8f * sinf(c*i/segments), 0, 0.8f * cosf(c*i/segments));
		end();
	}
};	// LineCircleComponent

class HistogamComponent : public DisplayListComponent
{
public:
	float func(float x) {
		return sinf(0.5f * x) * 10;
	}

	sal_override void render(void* context)
	{
		const float dt = (float)timer.get().asSecond();

		const size_t segments = 200;
		const float min = -12.5f;
		const float max = 12.5f;
		const float dx = (max - min) / segments;

		clear();
		begin(DisplayListComponent::Triangles);
			color(0, 0.5f, 0);

			for(float x = min; x <= max; x += dx) {
				const float y = func(x + dt);
				const float y2 = func(x + dx + dt);
				vertex(x, 0, 0);
				color(0, 1, 0);
				vertex(x, y, 0);
				color(0, 0.5f, 0);
				vertex(x+dx, 0, 0);

				vertex(x+dx, 0, 0);
				vertex(x, y, 0);
				vertex(x+dx, y2, 0);
			}
		end();

		DisplayListComponent::render(context);
	}

	Timer timer;
};	// HistogamComponent

}	// namespace

TEST(DisplayListTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=DisplayListTest;width=800;height=600;fullscreen=0;FSAA=8"));

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	ResourceManager& resourceManager = framework.resourceManager();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();
	CameraComponent* sceneCamera = scene.findComponentInChildrenExactType<CameraComponent>();
	sceneCamera->entity()->localTransform.translateBy(Vec3f(0, 10, 10));

	Entity* noLighting = scene.addChild(new Entity("No lighting material"));

	{	// Material for rendering line
		MaterialComponent* m = noLighting->addComponent(new MaterialComponent);
		m->lighting = false;
		m->cullFace = false;
		m->useVertexColor = true;
	}

	{	// Axis line
		Entity* e = noLighting->addChild(new Entity("Axis"));
		e->addComponent(new DrawAxisComponent);
	}

	{	// Dotted circles
		Entity* e = noLighting->addChild(new Entity("Dotted circles"));

		for(int i=-10; i<10; ++i) for(int j=-10; j<10; ++j)
		{
			Entity* e2 = e->addChild(new Entity);
			e2->localTransform.translateBy(Vec3f(float(i)* 3, 0, float(j) * 3));
			e2->addComponent(new LineCircleComponent);
		}
	}

	{	// Histogram
		Entity* e = noLighting->addChild(new Entity("Histogram"));
		e->addComponent(new HistogamComponent);

		// Equation label
		e = noLighting->addChild(new Entity("Equation label"));
		BmpFontMaterialComponent* m = e->addComponent(new BmpFontMaterialComponent);
		m->bmpFont = dynamic_cast<BmpFont*>(resourceManager.load("buildin/Arial-20.fnt").get());
		m->color = ColorRGBAf(1, 1, 0, 1);

		e = e->addChild(new Entity);
		e->localTransform.setTranslation(Vec3f(0, 12, 0));
		e->localTransform.scaleBy(Vec3f(0.05f));
		TextLabelComponent* text = e->addComponent(new TextLabelComponent);
		text->text = "y = 10 * sin(x/2 + t)";
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		renderer->render(root);
	}

	CHECK(true);
}
