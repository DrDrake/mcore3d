#ifndef __MCD_BINDING_PHYSICSCOMPONENT__
#define __MCD_BINDING_PHYSICSCOMPONENT__

#include "ShareLib.h"
#include "../Component/Physics/CollisionShape.h"
#include "../Component/Physics/ThreadedDynamicWorld.h"
#include "../../3Party/jkbind/Declarator.h"

namespace script {

SCRIPT_CLASS_DECLAR_EXPORT(MCD::CollisionShape, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::SphereShape, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::StaticPlaneShape, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::StaticTriMeshShape, MCD_BINDING_API);

SCRIPT_CLASS_DECLAR_EXPORT(MCD::RigidBodyComponent, MCD_BINDING_API);

SCRIPT_CLASS_DECLAR_EXPORT(MCD::DynamicsWorld, MCD_BINDING_API);
SCRIPT_CLASS_DECLAR_EXPORT(MCD::ThreadedDynamicsWorld, MCD_BINDING_API);

}	// namespace script

#endif	// __MCD_BINDING_PHYSICSCOMPONENT__
