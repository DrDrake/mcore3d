#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/TangentSpaceBuilder.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"

#include <fstream>

using namespace MCD;

//--------------------------------------------------------------------------
namespace RayMeshIntersectTest
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=RayMeshIntersectTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem()),
			mAccumTime(0)
		{
			// Override the default loader of *.3ds file
			mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

			// load scene
			const wchar_t* scenePath = L"Scene/AmbientCubeScene/AmbientCubeScene.3DS";
			EntityPrototypeLoader::addEntityAfterLoad(&mRootNode, mResourceManager, scenePath, 0, 0, L"loadAsEntity=true;editable=true");

			// disable Lighting
			glDisable(GL_LIGHTING);
		}

		sal_override void update(float deltaTime)
		{
			// update mAccumTime;
			mAccumTime += deltaTime;

			// remember to process loading events
			mResourceManager.processLoadingEvents();

			// update the transform
			//const float ct = cosf(mAccumTime * 0.5f);
			//const float st = sinf(mAccumTime * 0.5f);

			// finally render
			RenderableComponent::traverseEntities(&mRootNode);
		}

		DefaultResourceManager mResourceManager;
		
		float mAccumTime;
		Entity mRootNode;

	};	// TestWindow

}	// namespace RayMeshIntersectTest

TEST(RayMeshIntersectTest)
{	
	RayMeshIntersectTest::TestWindow window;
	window.mainLoop();
}
