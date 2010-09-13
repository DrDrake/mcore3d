#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Loader/PodLoader.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/RenderTargetComponent.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(PodLoaderTest)
{
	DeltaTimer timer;
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=PodLoaderTest;width=800;height=600;fullscreen=0;FSAA=4"));

	const char* model = "Scene/City/scene.pod";
//	const char* model = "Scene/scene.pod";

	Entity& root = framework.rootEntity();
	Entity& scene = framework.sceneLayer();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();

	{	// Register the pod loader
		framework.addLoaderFactory(new PodLoaderFactory);
	}

	{	// Setup GUI layer
		Entity* e = framework.guiLayer().addChild(new Entity("Text"));
		e->localTransform.setTranslation(Vec3f(0, 600, 0));
		TextLabelComponent* text = e->addComponent(new TextLabelComponent);
		text->text = model;
	}

	{	// Load the model using prefab
		Entity* e = new Entity("Model prefab");
		scene.addChild(e);
		e->localTransform.setScale(Vec3f(0.2f));
		CHECK(framework.loadPrefabTo(model, e));
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		const float dt = (float)timer.getDelta().asSecond();
		BehaviourComponent::traverseEntities(&root, dt);
		renderer->render(root);
	}

	CHECK(true);
}
