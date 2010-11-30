#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/Sprite.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Timer.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(SpriteTest)
{
	Framework framework;
	CHECK(framework.addFileSystem("Media"));
	CHECK(framework.initWindow("title=DisplayListTest;width=800;height=600;fullscreen=0;FSAA=8"));

	ResourceManager& resourceManager = framework.resourceManager();

	Entity& scene = framework.sceneLayer();
	Entity& gui = framework.guiLayer();
	CameraComponent* sceneCamera = scene.findComponentInChildrenExactType<CameraComponent>();
	sceneCamera->entity()->localTransform.translateBy(Vec3f(0, 10, 10));

	{
		Entity* e = gui.addFirstChild("Bear");
		SpriteAtlasComponent* atlas = e->addComponent(new SpriteAtlasComponent);
		atlas->textureAtlas = resourceManager.loadAs<Texture>("AnimBear.png");

		SpriteComponent* sprite = e->addComponent(new SpriteComponent);
		sprite->width = 512;
		sprite->height = 1024;
		sprite->uv = Vec4f(0, 0, 1, 1);
	}

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;
	}

	CHECK(true);
}
