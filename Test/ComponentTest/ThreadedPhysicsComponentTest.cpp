#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/InstancedMesh.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Physics/CollisionShape.h"
#include "../../MCD/Component/Physics/ThreadedDynamicWorld.h"
#include "../../MCD/Component/Physics/RigidBodyComponent.h"

using namespace MCD;

//#define USE_HARDWARE_INSTANCE

// InstancedMeshComponent is a renderable
// When it is visited by Renderable Vistor, it does not render anything, but register the per-instance info to the InstancedMesh
// The InstancedMesh then later render the geometry
class InstancedMeshComponent : public RenderableComponent
{
	InstancedMeshPtr mInstMesh;

public:
	InstancedMeshComponent(InstancedMesh& instMesh) : mInstMesh(&instMesh)
	{
	}

	sal_override void render()
	{
		renderFaceOnly();
	}

	sal_override void renderFaceOnly()
	{
		if(Entity* e = entity())
			mInstMesh.getNotNull()->registerPerInstanceInfo(e->localTransform);
	}
};	// InstancedMeshComponent

TEST(ThreadedPhysicsComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		class MyLoadCallback : public EntityPrototypeLoader::LoadCallback
		{
		public:
			MyLoadCallback(DynamicsWorld& dynamicsWorld) : mDynamicsWorld(dynamicsWorld) {}

			sal_override void doCallback()
			{
				// Call the parent's doCallback() first
				LoadCallback::doCallback();

				// Create physics components
				if(addToHere)
					createStaticRigidBody(mDynamicsWorld, *addToHere);
			}

		protected:
			DynamicsWorld& mDynamicsWorld;
		};	// MyLoadCallback

		TestWindow()
			:
			BasicGlWindow("title=ThreadedPhysicsComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			mRootNode.name = "root";

			// A scene node is added such that MyLoadCallback will not try to create rigid body for other Entities.
			mSceneNode = new Entity();
			mSceneNode->asChildOf(&mRootNode);

			// Override the default loader of *.3ds file
			mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

			{	// Use a 3ds mesh as the ground
				std::auto_ptr<MyLoadCallback> callback(new MyLoadCallback(mDynamicsWorld));
				EntityPrototypeLoader::addEntityAfterLoad(
					mSceneNode,
					mResourceManager,
					"Scene/City/scene.3ds",
					callback.get()
				);
				callback.release();
			}

			// The maximum random displacement added to the balls
			static const float randomness = 0.5f;

			mDynamicsWorld.setGravity(Vec3f(0, -5, 0));

			Vec3f ballInitialPosition(0, 290, 0), ballPosXDelta(10, 0, 0), ballPosYDelta(0, 0, 10);

			// Setup the chamfer box mesh
			MeshPtr ballMesh = new Mesh("");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 2);
			MCD_VERIFY(ballMesh->create(chamferBoxBuilder, Mesh::Static));

			mBallInstMesh = new InstancedMesh(ballMesh, static_cast<Effect*>(mResourceManager.load("Material/hwinst.fx.xml").get()));

			// Ball count
			int xCount = 32, yCount = 32;

			//  Setup a stack of balls
			for(int x = 0; x < xCount; ++x)
			{
				Vec3f ballPosition(ballInitialPosition + (float)(x - xCount / 2) * ballPosXDelta - (float)(yCount / 2) * ballPosYDelta);
				for(int y = 0; y < yCount; ++y)
				{	// Build entity
					std::auto_ptr<Entity> e(new Entity);
					e->name = "Shpere";
					e->localTransform = Mat44f(Mat33f::makeXYZRotation(0, Mathf::cPiOver4(), 0));

					// Add some randomness
					Vec3f randomOffset((Mathf::random() - 0.5f) * 2, (Mathf::random() - 0.5f) * 2, (Mathf::random() - 0.5f) * 2);
					e->localTransform.setTranslation(ballPosition + randomness * randomOffset);
					ballPosition += ballPosYDelta;

					// Add component
#ifdef USE_HARDWARE_INSTANCE
					InstancedMeshComponent* c = new InstancedMeshComponent(*mBallInstMesh);
					e->addComponent(c);
#else
					MeshComponent* c = new MeshComponent;
					c->mesh = ballMesh;
					c->effect = dynamic_cast<Effect*>(mResourceManager.load("Material/simple.fx.xml").get());
					e->addComponent(c);
#endif

					// Create the phyiscs component
					RigidBodyComponent* rbc = new RigidBodyComponent(mDynamicsWorld, 0.5f, new SphereShape(1));
					e->addComponent(rbc);

					e->asChildOf(&mRootNode);
					e.release();
				}
			}

			// Override camera position to see the city from the top
//			mCamera.position = Vec3f(0, 300, 0);
//			mCamera.lookAt = Vec3f(0, -1, 0);
//			mCamera.upVector = Vec3f(0, 0, 1);

			// Start the physics thread
			mPhysicsThread.start(mDynamicsWorld, false);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			BehaviourComponent::traverseEntities(&mRootNode, deltaTime);
			RenderableComponent::traverseEntities(&mRootNode);

#ifdef USE_HARDWARE_INSTANCE
			Mat44f viewMat;
			mCamera.computeView(viewMat.getPtr());
			mBallInstMesh->update(viewMat);
#endif
		}

		virtual ~TestWindow()
		{
			// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
			while(Entity* child = mRootNode.firstChild())
				child->destroyThis();
		}

		Entity mRootNode;
		EntityPtr mSceneNode;
		InstancedMeshPtr mBallInstMesh;
		ThreadedDynamicsWorld mDynamicsWorld;
		DefaultResourceManager mResourceManager;
		Thread mPhysicsThread;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
