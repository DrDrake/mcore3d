#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
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
	public:
		TestWindow()
			:
			BasicGlWindow("title=SimplePhysicsComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			// Setup the sphere mesh
			MeshPtr mesh = new Mesh;
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			MCD_VERIFY(mesh->create(chamferBoxBuilder, Mesh::Static));

			// Setup the collision mesh
			CollisionShapePtr sphereShape = new SphereShape(1);

			// Create the physics world
			mDynamicsWorld.setGravity(Vec3f(0, -5, 0));

			for(int b = 0; b < 2; ++b)
			{	// Setup balls
				std::auto_ptr<Entity> e(new Entity);
				e->name = "ChamferBox 1";
				if (b == 0)
					e->localTransform.setTranslation(Vec3f(-1.5, 5, 0));
				else
					e->localTransform.setTranslation(Vec3f(1.5, 4, 0));

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = dynamic_cast<Effect*>(mResourceManager.load("Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* rbc = new RigidBodyComponent(mDynamicsWorld, 0.5f, sphereShape);
				e->addComponent(rbc);

				e->asChildOf(&mRootNode);
				e.release();
			}

			{	// Setup the ground
				std::auto_ptr<Entity> e(new Entity);
				e->name = "Ground";
				e->localTransform.setTranslation(Vec3f(0, 0, 0));

				// Setup the ground mesh
				MeshPtr meshGround = new Mesh;
				PlaneMeshBuilder pBuilder(100, 100, 2, 2);
				MCD_VERIFY(meshGround->create(pBuilder, Mesh::Static));

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = meshGround;
				c->effect = dynamic_cast<Effect*>(mResourceManager.load("Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* rbc = new RigidBodyComponent(mDynamicsWorld, 0, new StaticPlaneShape(Vec3f::c010, 0));
				e->addComponent(rbc);

				e->asChildOf(&mRootNode);
				e.release();
			}
			
			// Override camera position to see the huge lattice of balls
			mCamera.position = Vec3f(0, 10, -3);
			mCamera.lookAt = Vec3f(0, -1, 0);
			mCamera.upVector = Vec3f(0, 0, 1);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			BehaviourComponent::traverseEntities(&mRootNode, deltaTime);
			RenderableComponent::traverseEntities(&mRootNode);

			mDynamicsWorld.stepSimulation(deltaTime, 10);
		}

		virtual ~TestWindow()
		{
			// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
			while(Entity* child = mRootNode.firstChild())
				child->destroyThis();
		}

		Entity mRootNode;
		DynamicsWorld mDynamicsWorld;
		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
