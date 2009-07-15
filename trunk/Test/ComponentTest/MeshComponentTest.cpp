#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/EntityPrototype.h"

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
			mEntPtt = dynamic_cast<EntityPrototype*>(mResourceManager.load(L"Scene/City/scene.3ds").get());

			std::auto_ptr<Entity> group1(new Entity);
			{	// Setup entity 1
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(group1.get());
				e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}

			{	// Setup entity 2
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(group1.get());
				e->localTransform.setTranslation(Vec3f(1, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}

			group1->localTransform.setTranslation(Vec3f(0, 1, 0));
			group1->asChildOf(&mRootNode);

			// clone a copy of group1
			std::auto_ptr<Entity> group2(group1->clone());

			group2->localTransform.setTranslation(Vec3f(0,-1, 0));
			group2->asChildOf(&mRootNode);

			// remember to release the auto_ptr :-)
			group1.release();
			group2.release();
		}

		sal_override void update(float deltaTime)
		{
			GLfloat lightPos[] = { 200, 200, 200, 1.0f };
			glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
			mResourceManager.processLoadingEvents();

			mRootNode.localTransform.setTranslation(Vec3f(0, 10, 0));
			RenderableComponent::traverseEntities(&mRootNode);

			if(nullptr != mEntPtt->mEntity.get())
				RenderableComponent::traverseEntities(mEntPtt->mEntity.get());
		}

		Entity mRootNode;
		EntityPrototypePtr mEntPtt;

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
