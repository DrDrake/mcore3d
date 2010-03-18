#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Loader/PodLoader.h"
#include "../../MCD/Component/Render/RenderableComponent.h"
#include "../../MCD/Component/Prefab.h"

using namespace MCD;

TEST(EntityPrototypeLoaderTest)
{

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
		mEntityPrototype = dynamic_cast<EntityPrototype*>(mResourceManager.load(fileId).get());

		mRootNode.localTransform.setScale(Vec3f(0.1f));
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

	TestWindow window;
	window.mainLoop();
}

#include "../../MCD/Component/PrefabLoaderComponent.h"

TEST(PrefabLoaderComponentTest)
{

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=PrefabLoaderComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		mResourceManager.addFactory(new PodLoaderFactory(mResourceManager));

		const wchar_t* fileId = L"Scene/City/scene.pod";
		EntityPtr e = PrefabLoaderComponent::loadEntity(mResourceManager, fileId, false);
		if(e)
			e->asChildOf(&mRootNode);

		mRootNode.localTransform.setScale(Vec3f(0.1f));
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();
		BehaviourComponent::traverseEntities(&mRootNode, deltaTime);
		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;
	DefaultResourceManager mResourceManager;
};	// TestWindow

	TestWindow window;
	window.mainLoop();
}
