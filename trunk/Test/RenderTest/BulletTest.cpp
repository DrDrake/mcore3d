#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "PlaneMeshBuilder.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../BulletBinding/Dynamics.h"
#include "../../BulletBinding/MathConvertor.h"

#include "../../3Party/bullet/btBulletCollisionCommon.h"
#include <ctime> 

using namespace MCD;
using namespace BulletBinding;

#ifdef NDEBUG
#	pragma comment(lib, "libbulletcollision")
#	pragma comment(lib, "libbulletmath")
#else
#	pragma comment(lib, "libbulletcollisiond")
#	pragma comment(lib, "libbulletmathd")
#endif

TEST(BulletTest)
{
	class TestWindow : public BasicGlWindow
	{
		std::auto_ptr<DynamicsWorld> dynamicsWorld;
		float mAbsTime;
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=BulletTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			// The maximum random displacement added to the balls
			static float randomness = 0.02f;
			srand((unsigned)time(0)); 

			mAbsTime = 0;

			dynamicsWorld.reset(new DynamicsWorld);
			dynamicsWorld->setGravity(Vec3f(0, -1, 0));

			Vec3f ballInitialPosition(-8, 2, 0), ballPosXDelta(2, 0, 0), ballPosYDelta(0, 4, 0);

			// Setup the chamfer box mesh
			MeshPtr mesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
			chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

			TriMeshShape* collisionMesh = new TriMeshShape(mesh);

			//  Setup a stack of balls
			for(int x = 0; x < 10; x++)
			{
				Vec3f ballPosition(ballInitialPosition + (float)x * ballPosXDelta);
				for(int y = 0; y < 50; y++)
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
					RigidBodyComponent* cc = new RigidBodyComponent(0.1f, collisionMesh);
					e->addComponent(cc);
					cc->onAttach();

					// Add it to the physics world..
					dynamicsWorld->addRigidBody(cc);

					e.release();
				}
			}

			{	// Setup the ground plane
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Floor";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(0, -1.5f, 0));

				// Setup the plane mesh
				MeshPtr mesh = new Mesh(L"");
				PlaneMeshBuilder planeBlder(100.0f, 100.0f, 2, 2);
				planeBlder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* cc = new RigidBodyComponent(
					0,
					new StaticPlaneShape(Vec3f(0, 1, 0), 0));
				e->addComponent(cc);
				cc->onAttach();

				// Add it to the physics world..
				dynamicsWorld->addRigidBody(cc);

				e.release();
			}

			// Override camera position to see the huge lattice of balls, hahahaha
			mCamera.position = Vec3f(0, 0, -30);
		}

		sal_override void update(float deltaTime)
		{
			static int calledTime = 0;
			mAbsTime += deltaTime;
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			RenderableComponent::traverseEntities(&mRootNode);
			BehaviourComponent::traverseEntities(&mRootNode);

			dynamicsWorld->stepSimulation(deltaTime, 10);
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
