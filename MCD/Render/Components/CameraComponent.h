#ifndef __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__
#define __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__

#include "RenderableComponent.h"
#include "../Camera.h"
#include "../Color.h"
#include "../../Core/System/Timer.h"

namespace MCD {

/*!
 */
class MCD_RENDER_API CameraComponent : public RenderableComponent
{
public:
	CameraComponent();

// Operations
	sal_override void render();

// Attrubutes
	Camera camera;
	Vec3f velocity;
	ColorRGBf clearColor;

private:
	// TODO: Remove the use of timer once the component system has a global timer.
	DeltaTimer mTimer;
};	// CameraComponent

typedef WeakPtr<CameraComponent> CameraComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__