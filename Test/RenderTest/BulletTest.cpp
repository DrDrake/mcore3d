#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../BulletBinding/Dynamics.h"

using namespace MCD;
using namespace BulletBinding;

#pragma comment(lib, "BulletBindingd")

TEST(BulletTest)
{
	class TestWindow : public BasicGlWindow
	{
		std::auto_ptr<DynamicsWorld> dynamicsWorld;
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=BulletTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{	// Setup entity 1
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(&mRootNode);
				e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));
				e->localTransform.setTranslation(Vec3f(-1.5, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				//c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* cc = new RigidBodyComponent(0.1f, new btSphereShape(1));
				e->addComponent(cc);

				// Add it to the physics world..
				dynamicsWorld.reset(new DynamicsWorld);
				dynamicsWorld->setGravity(Vec3f(0, -1, 0));
				dynamicsWorld->addRigidBody(cc);

				e.release();
			}

			{	// Setup entity 2
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(1.5, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				//c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			RenderableComponent::traverseEntities(&mRootNode);
			BehaviourComponent::traverseEntities(&mRootNode);

			dynamicsWorld->setGravity(-dynamicsWorld->getGravity());

			dynamicsWorld->stepSimulation(deltaTime,10);
		}

		Entity mRootNode;

		DefaultResourceManager mResourceManager;

		virtual ~TestWindow()
		{
			// Make sure the dynamics world is freed BEFORE the RigidBodyComponent...
			dynamicsWorld.reset();
		}
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
