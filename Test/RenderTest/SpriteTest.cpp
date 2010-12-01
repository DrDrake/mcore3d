#include "Pch.h"
#include "../../MCD/Framework/Framework.h"
#include "../../MCD/Render/Sprite.h"
#include "../../MCD/Core/Entity/BehaviourComponent.h"
#include "../../MCD/Core/Entity/Entity.h"

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

		// Texture source from:
		// http://www.raywenderlich.com/1271/how-to-use-animations-and-sprite-sheets-in-cocos2d
		atlas->textureAtlas = resourceManager.loadAs<Texture>("AnimBear.png");

		SpriteComponent* sprite = e->addComponent(new SpriteComponent);

		const float w = 233;
		const float h = 145;
		sprite->width = w;
		sprite->height = h;
		sprite->uv = Vec4f(0, 0, 1, 1);
		sprite->uv = Vec4f(1, 1, 1+w, 1+h);
	}

	framework.mainLoop();
	CHECK(true);
}
