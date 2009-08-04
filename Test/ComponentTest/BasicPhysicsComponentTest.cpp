#include "Pch.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Component/Physics/CollisionShape.h"
#include "../../MCD/Component/Physics/ThreadedDynamicWorld.h"
#include "../../MCD/Component/Physics/RigidBodyComponent.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Mesh.h"

using namespace MCD;

//! Test about the creation and destruction of rigid bodies
TEST(Create_BaiscPhysicsComponentTest)
{
	Entity rootNode;
	DynamicsWorld dynamicsWorld;

	{	ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
		StaticTriMeshShapePtr meshShape = new StaticTriMeshShape(chamferBoxBuilder);

		std::auto_ptr<Entity> e(new Entity);
		RigidBodyComponent* rbc = new RigidBodyComponent(dynamicsWorld, 0.5f, meshShape);
		e->addComponent(rbc);
		e->asChildOf(&rootNode);
		e.release();
	}

	CollisionShapePtr sphereShape = new SphereShape(1);
	for(size_t i=0; i<50; ++i)
	{	std::auto_ptr<Entity> e(new Entity);
		RigidBodyComponent* rbc = new RigidBodyComponent(dynamicsWorld, 0.5f, sphereShape);
		e->addComponent(rbc);
		e->asChildOf(&rootNode);
		e.release();
	}

	// Step the simulation
	for(size_t i=0; i<100; ++i) {
		dynamicsWorld.stepSimulation(0.1f, 10);
		BehaviourComponent::traverseEntities(&rootNode);
	}

	// Step the simulation as well as removing objects
	while(rootNode.firstChild()) {
		delete rootNode.firstChild();
		dynamicsWorld.stepSimulation(0.1f, 10);
		BehaviourComponent::traverseEntities(&rootNode);
	}
}

TEST(CreateThreaded_BaiscPhysicsComponentTest)
{
	Entity rootNode;
	ThreadedDynamicsWorld dynamicsWorld;
	Thread physicsThread;
	physicsThread.start(dynamicsWorld, false);

	{	ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
		StaticTriMeshShapePtr meshShape = new StaticTriMeshShape(chamferBoxBuilder);

		std::auto_ptr<Entity> e(new Entity);
		RigidBodyComponent* rbc = new RigidBodyComponent(dynamicsWorld, 0.5f, meshShape);
		e->addComponent(rbc);
		e->asChildOf(&rootNode);
		e.release();
	}

	CollisionShapePtr sphereShape = new SphereShape(1);
	for(size_t i=0; i<50; ++i)
	{	std::auto_ptr<Entity> e(new Entity);
		RigidBodyComponent* rbc = new RigidBodyComponent(dynamicsWorld, 0.5f, sphereShape);
		e->addComponent(rbc);
		e->asChildOf(&rootNode);
		e.release();
	}

	// Update components
	for(size_t i=0; i<100; ++i)
		BehaviourComponent::traverseEntities(&rootNode);

	// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
	while(rootNode.firstChild())
		delete rootNode.firstChild();
}
