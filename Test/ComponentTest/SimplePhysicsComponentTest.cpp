#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/PlaneMeshBuilder.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Physics/CollisionShape.h"
#include "../../MCD/Component/Physics/DynamicsWorld.h"
#include "../../MCD/Component/Physics/RigidBodyComponent.h"

using namespace MCD;

TEST(SimplePhysicsComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
		std::auto_ptr<DynamicsWorld> mDynamicsWorld;
		CollisionShapePtr			 mSphereShape;

		float mAbsTime;

	public:
		TestWindow()
			:
			BasicGlWindow(L"title=SimplePhysicsComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			// Setup the sphere mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			// Setup the collision mesh
			mSphereShape = new SphereShape(1);

			// Create the physics world
			mDynamicsWorld.reset(new DynamicsWorld);
			mDynamicsWorld->setGravity(Vec3f(0, -5, 0));

			for(int b = 0; b < 2; ++b)
			{	// Setup balls
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(&mRootNode);
				if (b == 0)
					e->localTransform.setTranslation(Vec3f(-1.5, 5, 0));
				else
					e->localTransform.setTranslation(Vec3f(1.5, 4, 0));

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* rbc = new RigidBodyComponent(0.5f, mSphereShape);
				e->addComponent(rbc);
				rbc->onAttach();
				// Add it to the physics world..
				mDynamicsWorld->addRigidBody(*rbc);

				e.release();
			}

			{	// Setup the ground
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(0, 0, 0));

				// Setup the sphere mesh
				MeshPtr meshGround = new Mesh(L"");
				PlaneMeshBuilder pBuilder(100, 100, 2, 2);
				pBuilder.commit(*meshGround, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = meshGround;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* rbc = new RigidBodyComponent(0, new StaticPlaneShape(Vec3f::c010, 0));
				e->addComponent(rbc);
				rbc->onAttach();
				// Add it to the physics world..
				mDynamicsWorld->addRigidBody(*rbc);

				e.release();
			}
			
			// Override camera position to see the huge lattice of balls, hahahaha
			mCamera.position = Vec3f(0, 10, -3);
			mCamera.lookAt = Vec3f(0, -1, 0);
			mCamera.upVector = Vec3f(0, 0, 1);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			static int calledTime = 0;
			mAbsTime += deltaTime;

			RenderableComponent::traverseEntities(&mRootNode);
			BehaviourComponent::traverseEntities(&mRootNode);

			mDynamicsWorld->stepSimulation(deltaTime, 10);
		}

		Entity mRootNode;

		virtual ~TestWindow()
		{
			// Make sure the dynamics world is freed BEFORE the RigidBodyComponent...
			mDynamicsWorld.reset();
		}

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
