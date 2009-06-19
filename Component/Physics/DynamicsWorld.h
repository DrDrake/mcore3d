#ifndef __MCD_COMPONENT_DYNAMICSWORLD__
#define __MCD_COMPONENT_DYNAMICSWORLD__

#include "../ShareLib.h"
#include "../../MCD/Core/System/NonCopyable.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

namespace PhysicsComponent {

class RigidBodyComponent;

class MCD_COMPONENT_API DynamicsWorld : Noncopyable
{
public:
	DynamicsWorld();

	~DynamicsWorld();

// Operations
	void addRigidBody(RigidBodyComponent* rbc);

	void stepSimulation(float timeStep, int maxSubStep);

// Attributes
	void setGravity(const Vec3f& g);
	Vec3f gravity() const;

private:
	class Impl;
	Impl* mImpl;
};	// DynamicsWorld

}	// PhysicsComponent

}	// MCD

#endif	// __MCD_COMPONENT_DYNAMICSWORLD__
