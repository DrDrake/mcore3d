#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/PickComponent.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

TEST(PickComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=PickComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{	// Setup the pick entity
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Picker";
				e->enabled = false;	// The picker is enable when there is mouse click
				e->asChildOf(&mRootNode);

				PickComponent* c = new PickComponent;
				c->entityToPick = &mRootNode;
				e->addComponent(c);
				mPicker = c;

				mPickerEntity = e.release();
			}

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
				c->effect = dynamic_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}

			{	// Setup entity 2
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(1, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = dynamic_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}
		}

		sal_override void onEvent(const Event& e)
		{
			BasicGlWindow::onEvent(e);

			if(e.Type != Event::MouseButtonReleased)
				return;

			mPickerEntity->enabled = true;
			mPicker->setPickRegion(e.MouseButton.X, e.MouseButton.Y);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			BehaviourComponent::traverseEntities(&mRootNode);
			RenderableComponent::traverseEntities(&mRootNode);

			// Show the picking result
			for(size_t i=0; i<mPicker->hitCount(); ++i) {
				EntityPtr e = mPicker->hitAtIndex(i);
				if(!e)
					continue;
				std::wcout << e->name << std::endl;
			}
			mPicker->clearResult();
			mPickerEntity->enabled = false;
		}

		Entity mRootNode;
		EntityPtr mPickerEntity;
		PickComponent* mPicker;

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
