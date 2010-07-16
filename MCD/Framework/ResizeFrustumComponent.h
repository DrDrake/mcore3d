#ifndef __MCD_FRAMEWORK_RESIZEFRUSTUMRCOMPONENT__
#define __MCD_FRAMEWORK_RESIZEFRUSTUMRCOMPONENT__

#include "ShareLib.h"
#include "../Core/Entity/BehaviourComponent.h"

namespace MCD {

typedef IntrusiveWeakPtr<class CameraComponent2> CameraComponent2Ptr;
typedef IntrusiveWeakPtr<class RenderTargetComponent> RenderTargetComponentPtr;

/*!	Makes a camera always up to date with a RenderTargetComponent's dimension
 */
class MCD_FRAMEWORK_API ResizeFrustumComponent : public BehaviourComponent
{
public:
	ResizeFrustumComponent();

	sal_override void update(float dt);

// Attributes
	CameraComponent2Ptr camera;
	RenderTargetComponentPtr renderTarget;

protected:
	size_t mOldWidth, mOldHeight;
};	// ResizeFrustumComponent

typedef IntrusiveWeakPtr<ResizeFrustumComponent> ResizeFrustumComponentPtr;

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_RESIZEFRUSTUMRCOMPONENT__
