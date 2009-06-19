#ifndef __MCD_COMPONENT_RENDER_MESHCOMPONENT__
#define __MCD_COMPONENT_RENDER_MESHCOMPONENT__

#include "RenderableComponent.h"
#include "../../MCD/Core/System/SharedPtr.h"

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;
typedef IntrusivePtr<class Effect> EffectPtr;

/*!
 */
class MCD_COMPONENT_API MeshComponent : public RenderableComponent
{
public:
	MeshComponent();

	sal_override ~MeshComponent();

// Operations
	sal_override void render();

	sal_override void renderFaceOnly();

// Attrubutes
	MeshPtr mesh;
	EffectPtr effect;
};	// MeshComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_MESHCOMPONENT__
