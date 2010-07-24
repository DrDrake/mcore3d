#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Component/Render/MeshComponent.h"

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow("title=MeshComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		std::auto_ptr<Entity> group1(new Entity);
		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = "ChamferBox 1";
			e->asChildOf(group1.get());
			e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh("");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			MCD_VERIFY(mesh->create(chamferBoxBuilder, Mesh::Static));

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = dynamic_cast<Effect*>(mResourceManager.load("Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}

		{	// Setup entity 2
			std::auto_ptr<Entity> e(new Entity);
			e->name = "Sphere 1";
			e->asChildOf(group1.get());
			e->localTransform.setTranslation(Vec3f(1, 0, 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh("");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			MCD_VERIFY(mesh->create(chamferBoxBuilder, Mesh::Static));

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = dynamic_cast<Effect*>(mResourceManager.load("Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}

		group1->localTransform.setTranslation(Vec3f(0, 2, 0));
		group1->asChildOf(&mRootNode);

		// Clone a copy of group1
		std::auto_ptr<Entity> group2(group1->clone());

		group2->localTransform.setTranslation(Vec3f(0, 0, 0));
		group2->asChildOf(&mRootNode);

		mRootNode.localTransform.setTranslation(Vec3f(0, 0, -10));

		// Remember to release the auto_ptr :-)
		group1.release();
		group2.release();
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();
		RenderableComponent::traverseEntities(&mRootNode);
	}

	Entity mRootNode;
	DefaultResourceManager mResourceManager;
};	// TestWindow

}	// namespace

TEST(MeshComponentTest)
{
	TestWindow window;
	window.mainLoop();
}
