#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Loader/Max3dsLoader.h"
#include "../../MCD/Loader/PodLoader.h"
#include "../../MCD/Render/Chamferbox.h"
#include "../../MCD/Render/Font.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"

using namespace MCD;

TEST(PodLoaderTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=PodLoaderTest;width=800;height=600;fullscreen=0;FSAA=4"));

	const char* model = "Scene/City/scene.pod";
//	const char* model = "3M00696/buelllightning.pod";
//	const char* model = "Scene/tv.pod";
//	const char* model = "Scene/City/scene.3ds";

	Entity& scene = framework.sceneLayer();

	{	// Register the pod loader
//		framework.addLoaderFactory(new Max3dsLoaderFactory);
		framework.addLoaderFactory(new PodLoaderFactory);
	}

	{	// Setup GUI layer
		Entity* e = framework.guiLayer().addFirstChild("Text");
		e->localTransform.setTranslation(Vec3f(0, 100, 0));
		TextLabelComponent* text = e->addComponent(new TextLabelComponent);
		text->text = model;
		text->anchor = Vec2f(0, 0);
	}

	{	// Load the model using prefab
		Entity* e = new Entity("Model prefab");
		scene.addFirstChild(e);
		e->localTransform.setScale(Vec3f(0.1f));
		CHECK(framework.loadPrefabTo(model, *e));
	}

	framework.mainLoop();
	CHECK(true);
}
