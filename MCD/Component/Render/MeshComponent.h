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
	sal_override sal_checkreturn bool cloneable() const { return true; }

	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();

	sal_override void renderFaceOnly();

// Attrubutes
	MeshPtr mesh;
	EffectPtr effect;
};	// MeshComponent

typedef WeakPtr<MeshComponent> MeshComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_MESHCOMPONENT__
