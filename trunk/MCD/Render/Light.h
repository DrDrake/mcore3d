#ifndef __MCD_RENDER_LIGHT__
#define __MCD_RENDER_LIGHT__

#include "Color.h"
#include "Renderable.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/StringHash.h"

namespace MCD {

class MCD_RENDER_API LightComponent : public RenderableComponent
{
public:
// Cloning
	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();
	sal_override void render2(sal_in void* context);

// Attributes
	FixString type;	//!< "point" or directional" or "spot"
	ColorRGBf color;

protected:
	sal_override ~LightComponent();
	ComponentPtr dummy;	// NOTE: Workaround for VC9 LNK1194 with vftable
};	// LightComponent

typedef IntrusiveWeakPtr<LightComponent> LightComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_LIGHT__
