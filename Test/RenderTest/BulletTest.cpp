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
			mAbsTime = 0;

			dynamicsWorld.reset(new DynamicsWorld);
			dynamicsWorld->setGravity(Vec3f(0, -1, 0));

			{	// Setup entity 1
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(&mRootNode);
				e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));
				e->localTransform.setTranslation(Vec3f(-1.5f, 3, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* cc = new RigidBodyComponent(0.1f, new btSphereShape(1));
				e->addComponent(cc);
				cc->onAttach();

				// Add it to the physics world..
				dynamicsWorld->addRigidBody(cc);

				e.release();
			}

			{	// Setup entity 2
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(1.5f, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 10);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				e.release();
			}

			{	// Setup a plane
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Floor";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(0, -1.5f, 0));

				// Setup the plane mesh
				MeshPtr mesh = new Mesh(L"");
				PlaneMeshBuilder planeBlder(10.0f, 10.0f, 1, 1);
				planeBlder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/test.fx.xml").get());
				e->addComponent(c);

				// Create the phyiscs component
				RigidBodyComponent* cc = new RigidBodyComponent(
					0,
					new btStaticPlaneShape(MathConvertor::ToBullet(Vec3f(0, 1, 0)), 0));
				e->addComponent(cc);
				cc->onAttach();

				// Add it to the physics world..
				dynamicsWorld->addRigidBody(cc);

				e.release();
			}
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
