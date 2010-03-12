#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Loader/PodLoader.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Component/Render/EntityPrototype.h"
#include "../../MCD/Component/Render/MeshComponent.h"

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=EntityPrototypeLoaderTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		mResourceManager.addFactory(new PodLoaderFactory(mResourceManager));
		const wchar_t* fileId = L"Scene/City/scene.pod";
//		const wchar_t* fileId = L"Scene/Sponza/scene2.pod";
//		const wchar_t* fileId = L"Scene/NPC/scene.pod";
//		const wchar_t* fileId = L"Scene/man.pod";
		mEntityPrototype = dynamic_cast<EntityPrototype*>(mResourceManager.load(fileId).get());

		mRootNode.localTransform.setScale(Vec3f(0.1f));
		mRootNode.localTransform.setTranslation(Vec3f(0, 0, -10));
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		// Detect whether the entity prototype is loaded, and move the entity tree to our root if so.
		if(mEntityPrototype && mEntityPrototype->entity.get()) {
			mEntityPrototype->entity->asChildOf(&mRootNode);
			mEntityPrototype->entity.release();
			mEntityPrototype = nullptr;
		}

		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;
	EntityPrototypePtr mEntityPrototype;
	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace

TEST(EntityPrototypeLoaderTest)
{
	TestWindow window;
	window.mainLoop();
}
