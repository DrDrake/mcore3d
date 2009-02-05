#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/Components/MeshComponent.h"

using namespace MCD;

TEST(MeshComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=MeshComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{	// Setup entity 1
				Entity* e = new Entity;
				e->name = L"ChamferBox 1";
				e->link(&mRootNode);
				e->localTransform = Mat44f::rotate(0, Mathf::cPiOver4(), 0);

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);
			}

			{	// Setup entity 2
				Entity* e = new Entity;
				e->name = L"Sphere 1";
				e->link(&mRootNode);
				e->localTransform.setTranslation(Vec3f(1, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);
			}
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			RenderableComponent::traverseEntities(&mRootNode);
		}

		Entity mRootNode;

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
