#ifndef __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__
#define __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__

#include "RenderableComponent.h"
#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Core/System/Timer.h"

namespace MCD {

/*!
 */
class MCD_COMPONENT_API CameraComponent : public RenderableComponent
{
public:
	CameraComponent();

// Operations
	sal_override void render();

// Attrubutes
	Camera camera;
	Vec3f velocity;
	ColorRGBf clearColor;
	// TODO: Move it to something like mouse camera controler
	bool isMouseDown;	//! To make associating the camera with mouse movement easily.

private:
	// TODO: Remove the use of timer once the component system has a global timer.
	DeltaTimer mTimer;
};	// CameraComponent

typedef WeakPtr<CameraComponent> CameraComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_COMPONENTS_CAMERACOMPONENT__