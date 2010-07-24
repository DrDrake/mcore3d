#ifndef __MCD_COMPONENTS_RENDER_CAMERACOMPONENT__
#define __MCD_COMPONENTS_RENDER_CAMERACOMPONENT__

#include "RenderableComponent.h"
#include "../../Render/Camera.h"
#include "../../Render/Color.h"

namespace MCD {

/*!
 */
class MCD_COMPONENT_API CameraComponent : public RenderableComponent
{
public:
	CameraComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();

// Attrubutes
	Camera camera;

	ColorRGBf clearColor;
};	// CameraComponent

typedef IntrusiveWeakPtr<CameraComponent> CameraComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENTS_RENDER_CAMERACOMPONENT__
