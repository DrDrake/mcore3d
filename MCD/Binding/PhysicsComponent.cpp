#include "Pch.h"
#include "PhysicsComponent.h"
#include "Entity.h"
#include "../Component/Physics/RigidBodyComponent.h"

using namespace MCD;

namespace script {

namespace types {

void addHandleToObject(HSQUIRRELVM v, CollisionShape* obj, int idx) {
	obj->scriptOwnershipHandle.setHandle(v, idx);
}

bool pushHandleFromObject(HSQUIRRELVM v, CollisionShape* obj) {
	return obj->scriptOwnershipHandle.vm && obj->scriptOwnershipHandle.pushHandle(v);
}

}	// namespace type

struct refPolicy {
	static void addRef(CollisionShape* s) {
		intrusivePtrAddRef(s);
	}
	static void releaseRef(CollisionShape* s) {
		intrusivePtrRelease(s);
	}
};	// refPolicy

SCRIPT_CLASS_REGISTER_NAME(CollisionShape, "CollisionShape")
;}

static SphereShape* sphereShapeCreate(float radius) {
	return new SphereShape(radius);
}

SCRIPT_CLASS_REGISTER(SphereShape)
	.declareClass<SphereShape, CollisionShape>(L"SphereShape")
	.staticMethod<constructObjRefCount<refPolicy> >(L"constructor", &sphereShapeCreate)
;}

SCRIPT_CLASS_REGISTER(StaticPlaneShape)
	.declareClass<StaticPlaneShape, CollisionShape>(L"StaticPlaneShape")
;}

SCRIPT_CLASS_REGISTER(StaticTriMeshShape)
	.declareClass<StaticTriMeshShape, CollisionShape>(L"StaticTriMeshShape")
;}

static RigidBodyComponent* rigidBodyComponentCreate(DynamicsWorld* dynamicsWorld, float mass, CollisionShape* collisionShape) {
	return new RigidBodyComponent(*dynamicsWorld, mass, collisionShape);
}

SCRIPT_CLASS_REGISTER(RigidBodyComponent)
	.declareClass<RigidBodyComponent, Component>(L"RigidBodyComponent")
	.staticMethod<construct>(L"constructor", &rigidBodyComponentCreate)
;}

SCRIPT_CLASS_REGISTER_NAME(DynamicsWorld, "DynamicsWorld")
;}

SCRIPT_CLASS_REGISTER(ThreadedDynamicsWorld)
	.declareClass<ThreadedDynamicsWorld, DynamicsWorld>(L"ThreadedDynamicsWorld")
;}

}	// namespace script

namespace MCD {

void registerPhysicsComponentBinding(script::VMCore* v)
{
	script::ClassTraits<CollisionShape>::bind(v);
	script::ClassTraits<SphereShape>::bind(v);
	script::ClassTraits<StaticPlaneShape>::bind(v);
	script::ClassTraits<StaticTriMeshShape>::bind(v);

	script::ClassTraits<RigidBodyComponent>::bind(v);

	script::ClassTraits<DynamicsWorld>::bind(v);
	script::ClassTraits<ThreadedDynamicsWorld>::bind(v);
}

}	// namespace MCD
