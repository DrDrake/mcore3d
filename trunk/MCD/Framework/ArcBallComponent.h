#ifndef __MCD_FRAMEWORK_ARCBALLCOMPONENT__
#define __MCD_FRAMEWORK_ARCBALLCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"
#include "../Core/Math/Vec2.h"

namespace MCD {

typedef IntrusiveWeakPtr<Entity> EntityPtr;
typedef IntrusiveWeakPtr<class InputComponent> InputComponentPtr;

/// A arc ball camera control.
/// Left click + mouse drag for roatation
/// Right click + mouse drag for focus panning
/// Mouse wheel for 
/// Reference: ARCBALL: A User Interface for Specifying Three-Dimensional Orientation Using a Mouse
/// http://www.tecgraf.puc-rio.br/~mgattass/fcg/material/shoemake92.pdf
/// http://rainwarrior.thenoos.net/dragon/arcball.html
class MCD_FRAMEWORK_API ArcBallComponent : public BehaviourComponent
{
public:
	ArcBallComponent();

	sal_override void update(float dt);

// Attributes
	EntityPtr target;	///< The entity that we want to control
	InputComponentPtr inputComponent;

	float radius;
	float minRadius;
	float rotationSpeed;
	float translationSpeed;
	Vec2f rotation;

protected:
	Vec2f mLastMouseAxis;
};	// ArcBallComponent

typedef IntrusiveWeakPtr<ArcBallComponent> ArcBallComponentPtr;

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_ARCBALLCOMPONENT__
