#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=MeshComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem())
	{
		// Override the default loader of *.3ds file
		mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

		const wchar_t* filepath = L"Scene/City/scene.3ds";

		EntityPrototypeLoader::addEntityAfterLoad(&mRootNode, mResourceManager, filepath);

		std::auto_ptr<Entity> group1(new Entity);
		{	// Setup entity 1
			std::auto_ptr<Entity> e(new Entity);
			e->name = L"ChamferBox 1";
			e->asChildOf(group1.get());
			e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
			commitMesh(chamferBoxBuilder, *mesh, Mesh::Static);

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
			e->asChildOf(group1.get());
			e->localTransform.setTranslation(Vec3f(1, 0, 0));

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			commitMesh(chamferBoxBuilder, *mesh, Mesh::Static);

			// Add component
			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = dynamic_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
			e->addComponent(c);

			e.release();
		}

		group1->localTransform.setTranslation(Vec3f(0, 11, 0));
		group1->asChildOf(&mRootNode);

		// Clone a copy of group1
		std::auto_ptr<Entity> group2(group1->clone());

		group2->localTransform.setTranslation(Vec3f(0, 9, 0));
		group2->asChildOf(&mRootNode);

		// Remember to release the auto_ptr :-)
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
