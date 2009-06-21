#include "Pch.h"
#include "../MCD/Binding/Binding.h"
#include "../MCD/Core/Entity/Entity.h"
#include "../MCD/Core/Entity/BehaviourComponent.h"
#include "../MCD/Render/ChamferBox.h"
#include "../MCD/Render/Effect.h"
#include "../MCD/Component/Render/MeshComponent.h"
#include "../Test/RenderTest/BasicGlWindow.h"
#include "../Test/RenderTest/DefaultResourceManager.h"

using namespace MCD;

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=MCD Launcher;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->asChildOf(&mRootNode);
			e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		BehaviourComponent::traverseEntities(&mRootNode);
		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;
	ScriptVM mScriptVM;
	DefaultResourceManager mResourceManager;
};	// TestWindow

int main()
{
	TestWindow window;

	window.mScriptVM.runScript(
		L""
	);
	window.mainLoop();

	return 0;
}
