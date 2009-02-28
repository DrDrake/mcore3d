#ifndef __MCD_RENDER_COMPONENTS_RENDERABLECOMPONENT__
#define __MCD_RENDER_COMPONENTS_RENDERABLECOMPONENT__

#include "../ShareLib.h"
#include "../../Core/Entity/Component.h"

namespace MCD {

/*!	The component family which is something renderable.
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API RenderableComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(RenderableComponent);
	}

	//! The derived components should override this function for rendering.
	virtual void render() = 0;

	//! The derived components should override this function for rendering, withou any material.
	virtual void renderFaceOnly() = 0;

	//! Invoke the RenderableComponent::render() in every Entity under the entityNode sub-tree.
	static void traverseEntities(sal_maybenull Entity* entityNode);
};	// RenderableComponent

}	// namespace MCD

#endif	// __MCD_RENDER_COMPONENTS_RENDERABLECOMPONENT__
