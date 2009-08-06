#ifndef __MCD_COMPONENT_RENDER_RENDERMODIFIERCOMPONENT__
#define __MCD_COMPONENT_RENDER_RENDERMODIFIERCOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/Component.h"

namespace MCD {

/*!	The component family performs per-entity render state overrides when RenderableComponents are render.
	An RenderableComponent may choose wether to support RenderModifierComponent or not; if RenderModifierComponent
	is support, the preGeomRender() and postGeomRender() method should be called prior and after the geometry is rendered,
	so that RenderModifierComponents can modify / restore the render states.
 */
class MCD_ABSTRACT_CLASS MCD_COMPONENT_API RenderModifierComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(RenderModifierComponent);
	}

	//! Invoked just after the renderstates are perpared; and before the geometry is rendered.
	virtual void preGeomRender(RenderModifierComponent& c) = 0;

	//! Invoked just after the geometry is rendered; and before the renderstates are un-perpared.
	virtual void postGeomRender(RenderModifierComponent& c) = 0;

	//! Invoke the RenderModifierComponent::render() in every Entity under the entityNode sub-tree.
	//static void traverseEntities(sal_maybenull Entity* entityNode);
};	// RenderModifierComponent

}	// namespace MCD

#endif	// __MCD_COMPONENT_RENDER_RENDERMODIFIERCOMPONENT__
