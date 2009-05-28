#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Render/TangentSpaceBuilder.h"


using namespace MCD;

TEST(NormalMappingTest)
{
	class TestWindow : public BasicGlWindow
	{
	private:
		ModelPtr mModel;

	public:
		TestWindow()
			:
			BasicGlWindow(L"title=NormalMappingTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{
				mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds", true).get());
			}
			{	// Setup entity 1
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(&mRootNode);
				e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
				TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);

				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());
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
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
				TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());
				e->addComponent(c);

				e.release();
			}
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			if(mModel)
				mModel->draw();

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
