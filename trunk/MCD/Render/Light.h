#ifndef __MCD_RENDER_LIGHT__
#define __MCD_RENDER_LIGHT__

#include "Color.h"
#include "../Core/Entity/Component.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/StringHash.h"

namespace MCD {

class MCD_RENDER_API LightComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(LightComponent);
	}

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }

	sal_override sal_notnull Component* clone() const { return new LightComponent; }

// Operations
	sal_override void render() {}

// Attributes
	FixString type;	//!< "point" or directional" or "spot"
	ColorRGBf color;
};	// LightComponent

typedef IntrusiveWeakPtr<LightComponent> LightComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_LIGHT__
