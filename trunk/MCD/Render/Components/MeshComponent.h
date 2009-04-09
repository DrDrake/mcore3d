#ifndef __MCD_RENDER_COMPONENTS_MESHCOMPONENT__
#define __MCD_RENDER_COMPONENTS_MESHCOMPONENT__

#include "RenderableComponent.h"
#include "../../Core/System/SharedPtr.h"

namespace MCD {

class Mesh;
typedef IntrusivePtr<Mesh> MeshPtr;

class Effect;
typedef IntrusivePtr<Effect> EffectPtr;

/*!	
 */
class MCD_RENDER_API MeshComponent : public RenderableComponent
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

#endif	// __MCD_RENDER_COMPONENTS_MESHCOMPONENT__
