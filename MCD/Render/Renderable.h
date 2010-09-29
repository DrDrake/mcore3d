#ifndef __MCD_RENDER_RENDERABLE__
#define __MCD_RENDER_RENDERABLE__

#include "ShareLib.h"
#include "../Core/System/Platform.h"
#include "../Core/Entity/Component.h"

namespace MCD {

/*!	A common interface that will make draw call.
	To simply renderer implementation.
 */
class MCD_ABSTRACT_CLASS IDrawCall
{
public:
	virtual ~IDrawCall() {}

	virtual void draw(sal_in void* context) = 0;
};	// IDrawCall

/*!	The component family which is something renderable.
 */
class MCD_ABSTRACT_CLASS MCD_RENDER_API RenderableComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(RenderableComponent);
	}

	sal_override void gather();

	//!	Invoked by RendererComponent
	virtual void render(sal_in void* context) = 0;
};	// RenderableComponent

typedef IntrusiveWeakPtr<RenderableComponent> RenderableComponentPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERABLE__
