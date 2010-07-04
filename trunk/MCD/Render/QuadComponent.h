#ifndef __MCD_RENDER_QUADCOMPONENT__
#define __MCD_RENDER_QUADCOMPONENT__

#include "Color.h"
#include "../Core/Entity/Component.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/StringHash.h"

namespace MCD {

/*!	Represent a quad to be render.
 */
class MCD_RENDER_API QuadComponent : public Component
{
public:
	QuadComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(QuadComponent);
	}

// Cloning
	sal_override sal_checkreturn bool cloneable() const;

	sal_override sal_notnull Component* clone() const;

// Operations
	sal_override void render();

// Attributes
	size_t width, height;

protected:
	sal_override ~QuadComponent();
};	// QuadComponent

typedef IntrusiveWeakPtr<QuadComponent> QuadComponentPtr;

//!	To preform actual (batched) rendering of QuadComponent
class MCD_RENDER_API QuadRendererComponent : public Component
{
public:
	QuadRendererComponent();

	sal_override const std::type_info& familyType() const {
		return typeid(QuadRendererComponent);
	}

// Operations
	sal_override void render();

	//!	Register a quad to render, ignored if the input is null.
	void renderQuad(sal_in_opt QuadComponent* quad);

protected:
	sal_override ~QuadRendererComponent();
};	// QuadRendererComponent

}	// namespace MCD

#endif	// __MCD_RENDER_QUADCOMPONENT__
