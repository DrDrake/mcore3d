#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/PlaneMeshBuilder.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Physics/CollisionShape.h"
#include "../../MCD/Component/Physics/DynamicsWorld.h"
#include "../../MCD/Component/Physics/RigidBodyComponent.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Max3dsLoader.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Core/System/PtrVector.h"

using namespace MCD;

TEST(PhysicsComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
		std::auto_ptr<DynamicsWorld>   mDynamicsWorld;
		std::auto_ptr<CollisionShape>  mBallCollisionMesh;
		float mAbsTime;
		ModelPtr mModel;
		ptr_vector<CollisionShape> mGroundCollisionMeshes;
	public:
		void processResourceLoadingEvents()
		{
			while(true) {
				ResourceManager::Event e = mResourceManager.popEvent();
				if(e.loader) {
					if(e.loader->getLoadingState() == IResourceLoader::Aborted)
						std::wcout << L"Resource:" << e.resource->fileId().getString() << L" failed to load" << std::endl;
					// Allow one resource to commit for each frame
					e.loader->commit(*e.resource);
				} else
					break;
			}
		}

		TestWindow()
			:
			BasicGlWindow(L"title=PhysicsComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			mResourceManager.addFactory(new Max3dsLoaderFactory(mResourceManager));

			// The maximum random displacement added to the balls
			static float randomness = 4.0f;

			mAbsTime = 0;

			mDynamicsWorld.reset(new DynamicsWorld);
			mDynamicsWorld->setGravity(Vec3f(0, -10, 0));

			Vec3f ballInitialPosition(0, 200, 0), ballPosXDelta(8, 0, 0), ballPosYDelta(0, 0, 8);

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			mBallCollisionMesh.reset(new SphereShape(1));

			int xCount = 30, yCount = 30;

			//  Setup a stack of balls
			for(int x = 0; x < xCount; ++x)
			{
				Vec3f ballPosition(ballInitialPosition + (float)(x - xCount / 2) * ballPosXDelta - (float)(yCount / 2) * ballPosYDelta);
				for(int y = 0; y < yCount; ++y)
				{	// Build entity
					std::auto_ptr<Entity> e(new Entity);
					e->name = L"ChamferBox 1";
					e->asChildOf(&mRootNode);
					e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));
					// Add some randomness, hehehehe
					
					Vec3f randomOffset(Mathf::random(), Mathf::random(), 0);
					e->localTransform.setTranslation(ballPosition + randomness * randomOffset);
					ballPosition += ballPosYDelta;

					// Add component
					MeshComponent* c = new MeshComponent;
					c->mesh = mesh;
					c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
					e->addComponent(c);

					// Create the phyiscs component
					RigidBodyComponent* cc = new RigidBodyComponent(0.1f, mBallCollisionMesh.get());
					e->addComponent(cc);
					cc->onAttach();

					// Add it to the physics world..
					mDynamicsWorld->addRigidBody(cc);

					e.release();
				}
			}

			{
				// Use a 3ds mesh as the ground
				mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds", true).get());
				processResourceLoadingEvents();

				for(Model::MeshList::iterator it = mModel->mMeshes.begin(); it != mModel->mMeshes.end(); it++)
				{
					MeshPtr mesh = it->mesh;

					std::auto_ptr<CollisionShape> collisionShape(new StaticTriMeshShape(mesh));

					// Setup the ground plane
					std::auto_ptr<Entity> e(new Entity);
					e->name = L"Floor";
					e->asChildOf(&mRootNode);
					e->localTransform.setTranslation(Vec3f(0, -1.5f, 0));

					// Add component
					MeshComponent* c = new MeshComponent;
					c->mesh = mesh;
					c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());
					e->addComponent(c);

					// Create the phyiscs component
					RigidBodyComponent* cc = new RigidBodyComponent(0, collisionShape.get());
					e->addComponent(cc);
					cc->onAttach();

					// Add it to the physics world..
					mDynamicsWorld->addRigidBody(cc);

					e.release();

					mGroundCollisionMeshes.push_back(collisionShape.release());
				}
			}

			// Override camera position to see the huge lattice of balls, hahahaha
			mCamera.position = Vec3f(0, 300, 0);
			mCamera.lookAt = Vec3f(0, -1, 0);
			mCamera.upVector = Vec3f(0, 0, 1);
		}

		sal_override void update(float deltaTime)
		{
			static int calledTime = 0;
			mAbsTime += deltaTime;
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			RenderableComponent::traverseEntities(&mRootNode);
			BehaviourComponent::traverseEntities(&mRootNode);

			mDynamicsWorld->stepSimulation(deltaTime, 10);
		}

		Entity mRootNode;

		DefaultResourceManager mResourceManager;

		virtual ~TestWindow()
		{
			// Make sure the dynamics world is freed BEFORE the RigidBodyComponent...
			mDynamicsWorld.reset();
		}
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
