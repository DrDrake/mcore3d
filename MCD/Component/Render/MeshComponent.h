#ifndef __MCD_COMPONENT_RENDER_MESHCOMPONENT__
#define __MCD_COMPONENT_RENDER_MESHCOMPONENT__

#include "RenderableComponent.h"
#include "../../Core/System/SharedPtr.h"

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

// Cloning
	sal_override bool cloneable() const { return false; }

	sal_override Component* clone() const { return nullptr; }

// Operations
	sal_override void render();

	sal_override void renderFaceOnly();

// Attrubutes
	MeshPtr mesh;
	EffectPtr effect;
};	// MeshComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_MESHCOMPONENT__
